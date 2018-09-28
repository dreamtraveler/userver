#include "tcp_conn.h" 
#include "tcp_server.h"
#include <cassert>
#include <stdio.h>
#include "gx.h"
#include "log.h"
#include "application.h"
#include "servlet.h"

namespace gx {

	uint32_t TCPConn::_genseq = 0;
	Buffer TCPConn::_send_tmp_buf;
	std::queue<write_req_t*> TCPConn::_write_req_queue;

	TCPConn::TCPConn(uint32_t id, TCPServer* server, uv_stream_t* server_stream) {
		_id = id;
		_state = ConnState::connected;
		init(server->loop());
		_server = server;
		int r = uv_accept(server_stream, (uv_stream_t*)(&_socket));
		return_if_err(r);

		struct sockaddr peername;
		int namelen = sizeof peername;
		char check_ip[17];
		uv_tcp_getpeername(&_socket, &peername, &namelen);
		struct sockaddr_in check_addr = *(struct sockaddr_in*) (&peername);
		uv_ip4_name(&check_addr, (char*)check_ip, sizeof check_ip);
		log_debug("new connection %s:%d", check_ip, check_addr.sin_port);

		r = uv_read_start((uv_stream_t*)(&_socket), alloc, after_read);
		return_if_err(r);
	}

	TCPConn::TCPConn(uv_loop_t* loop) {
		_id = TCPServer::genid();
		init(loop);
	}

	void TCPConn::init(uv_loop_t *loop) {
		int r = uv_tcp_init(loop, &_socket);
		return_if_err(r);

		/* associate this with stream */
		_socket.data = this;

		_read_buf = uv_buf_init(_inner_buf, READ_BUF_SIZE);
		_connect_req.data = this;
	}

	TCPConn::~TCPConn() {
		log_debug("~TCPConn()");
	}


	void TCPConn::alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		if (!handle->data) {
			return;
		}
		TCPConn *conn = (TCPConn*)handle->data;
		*buf = conn->_read_buf;
	}

	void TCPConn::after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
		TCPConn* conn = (TCPConn*)handle->data;
		if (nread < 0) {
			if (nread == UV_EOF) {
				log_info("disconnect by peer, %s: %s", uv_err_name(nread), uv_strerror(nread));
			}
			else if (nread == UV_ECONNRESET) {
				log_info("connection reset by peer, %s: %s", uv_err_name(nread), uv_strerror(nread));
			}
			else {
				log_error("except disconnect, %s: %s", uv_err_name(nread), uv_strerror(nread));
			}
			conn->close();
		}
		else if (nread == 0) {
			/* Everything OK, but nothing read. */
			return;
		}
		else {
			conn->on_recv(buf->base, nread);
		}
	}

	void TCPConn::on_recv(char* buf, size_t nread) {
		_read_msg_buf.write(buf, nread);
		uint32_t len = 0;
		int r = 0;
		while (_read_msg_buf.length() >= 10) {
			len = _read_msg_buf.peek_uint32();
			if (len < 4) {
				log_error("msg len < 4");
				this->close();
				return;
			}
			if (len > _read_msg_buf.length()) {
				break;
			}
			_read_msg_buf.skip(4);
			uint32_t seq = _read_msg_buf.read_uint32();
			uint16_t msgid = _read_msg_buf.read_uint16();
			int servletid = msgid >> 12;
			int r;
			if (_server && servletid == _server->id()) {
				r = on_request(msgid, seq, &_read_msg_buf);
			}
			else {
				r = on_response(msgid, seq, &_read_msg_buf);
			}

			if (r > 0) {
				_read_msg_buf.reset();
			}
			else if (r<0) {
				close();
			}
		}

		if (_read_msg_buf.length() > 0) {
			if (_read_msg_buf.on_starting_point()) {
				return;
			}
			else {
				auto butt = _read_msg_buf.next_all();
				_read_msg_buf.append(butt);
			}
		}

		return;
	}

	int TCPConn::on_request(uint16_t msgid, uint32_t seq, Buffer* buf) {
		return ServletMgr::instance()->execute(shared_from_this(), seq, msgid, buf);
	}

	int TCPConn::on_response(uint16_t msgid, uint32_t seq, Buffer* buf) {
		return ServletMgr::instance()->execute_response(seq, msgid, buf);
	}

	void TCPConn::connect(const char* ip, int port) {
		State(ConnState::connecting);
		struct sockaddr_in bind_addr;
		int r = uv_ip4_addr(ip, port, &bind_addr);
		return_if_err(r);

		r = uv_tcp_connect(&_connect_req, &_socket, (const sockaddr*)&bind_addr, after_connect);
		return_if_err(r);
	}

	void TCPConn::after_connect(uv_connect_t* handle, int status) {
		TCPConn *pclient = (TCPConn*)handle->handle->data;
		int r = uv_read_start(handle->handle, TCPConn::alloc, TCPConn::after_read);
		return_if_err(r);

		if (status == 0) {
			pclient->State(ConnState::connected);
			if (pclient->_connect_ok_handler != nullptr) {
				pclient->_connect_ok_handler();
			}
			log_debug("connect Ok");
		}
		else {
			if (pclient->_connect_fail_handler != nullptr) {
				pclient->_connect_fail_handler();
			}
			log_error("connect Fail, %s:%s", uv_err_name(status), uv_strerror(status));
		}
	}

	bool TCPConn::send(const char *buf, size_t nbuf) {
		if (_state != ConnState::connected) {
			log_error("socket no connected!!!");
			return false;
		}

		write_req_t* wr = pop_write_req();
		wr->req.data = this;
		wr->buf.base = (char*)buf;
		wr->buf.len = nbuf;
		int r = uv_write(&wr->req, (uv_stream_t*)(&_socket), &wr->buf, 1, TCPConn::after_write);
		return (0 == r);
	}

	void TCPConn::after_write(uv_write_t* req, int status) {
		write_req_t* wr = (write_req_t*)req;
		TCPConn* conn = (TCPConn*)(wr->req.handle->data);
		push_write_req(wr);

		if (status < 0) {
			if (!uv_is_closing((uv_handle_t*)req->handle)) {
				conn->close();
			} else {
				return_if_err(status);
			}
		}
	}

	void TCPConn::close() {
		uv_handle_t* handle = (uv_handle_t*)(&_socket);
		if (uv_is_closing(handle) == 0) {
			_state = ConnState::closed;
			log_debug("conn begin close, id=%d", _id);
			uv_close((uv_handle_t*)(&_socket), TCPConn::after_close);
		}
	}

	void TCPConn::after_close(uv_handle_t *handle) {
		TCPConn* conn = (TCPConn*)handle->data;
		log_debug("conn after close, id=%d", conn->id());
		bool pasv = (conn->_server != nullptr);
		if (conn->_disconnect_handler != nullptr) {
			conn->_disconnect_handler();
		}
		if (pasv) {
			conn->_server->remove_conn(conn->id());
		}
	}

	Slice TCPConn::pack(Buffer& buf, ISerial* req, uint32_t seq) {
		buf.reset();
		buf.append_uint32(seq);
		buf.append_uint16(req->msgid());
		req->serial(&buf);
		buf.prepend_uint32((uint32_t)buf.length() + 4);
		return buf.toslice();
	}

	bool TCPConn::send(uint32_t seq, ISerial* req) {
		_send_tmp_buf.reset();
		Slice slice = std::move(pack(_send_tmp_buf, req, seq));
		return send(slice.data(), slice.size());
	}

	write_req_t* TCPConn::pop_write_req() {
		if (_write_req_queue.size() > 0) {
			write_req_t* tmp = _write_req_queue.front();
			_write_req_queue.pop();
			return tmp;
		}
		else {
			return new write_req_t();
		}
	}

	int TCPConn::write_req_count() {
		return _write_req_queue.size();
	}

	void TCPConn::push_write_req(write_req_t* wreq) {
		if (_write_req_queue.size() < WRITE_REQ_QUEUE_SIZE) {
			_write_req_queue.push(wreq);
		}
		else {
			delete wreq;
		}
	}
}

