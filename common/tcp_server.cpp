#include "tcp_conn.h"
#include "tcp_server.h"
#include "gx.h"
#include "log.h"

namespace gx {
	uint32_t TCPServer::_gid = 0;

	TCPServer::TCPServer(int id) :_id(id) {
	}
	TCPServer::~TCPServer() {
	}

	void TCPServer::on_connection(uv_stream_t* server_stream, int err) {
		if (err != 0) {
			log_error("Connect error %s", uv_err_name(err));
		}

		TCPServer* server = (TCPServer*)server_stream->data;
		_gid++;
		server->add_conn(std::make_shared<TCPConn>(_gid, server, server_stream));
	}

	uint32_t TCPServer::genid() {
		_gid++;
		return _gid;
	}

	bool TCPServer::listen(const char* ip, int port) {
		log_info("listen on %s: %d", ip, port);
		_loop = uv_default_loop();
		struct sockaddr_in addr;

		int r;
		r = uv_ip4_addr(ip, port, &addr);
		if (r > 0) { return false; }

		r = uv_tcp_init(_loop, &_socket);
		if (r) {
			log_error("Socket creation error %s", uv_err_name(r));
			return false;
		}

		r = uv_tcp_bind(&_socket, (const struct sockaddr*) &addr, 0);
		if (r) {
			log_error("Bind error %s", uv_err_name(r));
			return false;
		}

		uv_stream_t* server_stream = (uv_stream_t*)&_socket;
		r = uv_listen(server_stream, SOMAXCONN, on_connection);
		if (r) {
			log_error("Listen error %s", uv_err_name(r));
			return false;
		}
		server_stream->data = this;
		return true;
	}

	bool TCPServer::set_nodelay(bool enable) {
		int r = uv_tcp_nodelay(&_socket, enable ? 1 : 0);
		if (r) {
			return false;
		}
		return true;
	}

	bool TCPServer::set_keepalive(int enable, unsigned int delay)
	{
		int r = uv_tcp_keepalive(&_socket, enable, delay);
		if (r) {
			return false;
		}
		return true;
	}
}
