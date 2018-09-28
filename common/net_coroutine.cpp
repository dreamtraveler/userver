#include "serial.h"
#include "tcp_conn.h"
#include "net_coroutine.h"

namespace gx {
	uint32_t NetCoroutine::_incrid=0;
	std::unordered_map<unsigned, std::shared_ptr<NetCoroutine>> NetCoroutine::_co_map;

	bool NetCoroutine::call(TCPConn* conn, IRpcMessage& msg) {
		static Buffer tmpbuf;
		tmpbuf.reset();
		msg.serial_request(&tmpbuf);
		if (conn->send(_id, msg.get_request())) {
			yield();

			co_param_t param = _sink->get();
			Buffer* rspbuf = std::get<1>(param);
			msg.set_response(rspbuf);
			return true;
		}
		else {
			finished(true);
			return false;
		}
	}
}
