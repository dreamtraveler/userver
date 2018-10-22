#include <utility>
#include "servlet.h"
#include "log.h"
#include "tcp_conn.h"
#include "net_coroutine.h"

namespace gx {

	ServletMgr ::ServletMgr() {
		_cofun = std::bind(&ServletMgr::co_execute, this, _1);
	}

	ServletMgr::~ServletMgr() {
		auto it = _servlet_map.begin();
		for (; it != _servlet_map.end(); ++it) {
			delete it->second;
		}
		_servlet_map.clear();
	}

	int ServletMgr::execute(std::shared_ptr<TCPConn> conn, uint32_t seq, uint16_t msgid, Buffer* buf) {
		auto it = _servlet_map.find(msgid);
		if (it == _servlet_map.end()) {
			log_error("no msgid, %x ", msgid);
			return -1;
		}
		if (it->second->use_coroutine()) {
			NetCoroutine* co = NetCoroutine::spawn(seq, _cofun, std::move(conn), it->second);
			co_param_t param(co->id(), buf);
			co->resume(param);
			if (co->finished()) {
				NetCoroutine::remove(co->id());
			}
		}
		else {
			auto req = it->second->create_request(buf);
			auto rsp = it->second->create_response();
			int r = it->second->sync_execute(std::move(conn), req.get(), rsp.get());
			if (r < 0) {
				conn->close();
			}
			else {
				conn->send(seq, rsp.get());
			}
		}
		return 0;
	}

	int ServletMgr::execute_response(uint32_t seq, uint16_t msgid, Buffer* buf) {
		auto co = NetCoroutine::find(seq);
		if (!co) {
			log_error("no callback coroutine, %x, %d ", msgid, seq);
			return 1;
		}

		co_param_t param(0, buf);
		co->resume(param);
		if (co->finished()) {
			NetCoroutine::remove(co->id());
		}
		return 0;
	}

	void ServletMgr::co_execute(co_pull_t &sink) {
		co_param_t param  = sink.get();
		uint32_t id = std::get<0>(param);
		Buffer* buf = std::get<1>(param);

		auto co = NetCoroutine::find(id);
		assert(co != nullptr);
		co->sink(&sink);

		ServletBase* servlet = co->servlet();
		auto req = servlet->create_request(buf);
		auto rsp = servlet->create_response();

		std::shared_ptr<TCPConn> conn = co->conn();
		int r = servlet->async_execute(conn, req.get(), rsp.get(), co);
		if (r < 0) {
			conn->close();
		}
		else {
			conn->send(co->seq(), rsp.get());
		}

		co->finished(true);
		return;
	}

}