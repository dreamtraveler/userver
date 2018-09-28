#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <string>
#include <unordered_map>
#include <memory>
#include "uv.h"
#include "platform.h"

namespace gx {

	class TCPConn;

	class TCPServer {
	public:
		TCPServer(int id);
		~TCPServer();

		TCPServer(const TCPServer&) = delete;
		TCPServer& operator=(const TCPServer&) = delete;

	private:
		static void on_connection(uv_stream_t* server, int status);

	public:
		static uint32_t genid();
		bool listen(const char* ip, int port);
		bool set_keepalive(int enable, unsigned int delay);
		bool set_nodelay(bool enable);
		void add_conn(std::shared_ptr<TCPConn>&& conn) {
			_connmap.emplace(conn->id(), conn);
		}
		void remove_conn(uint32_t id) {
			_connmap.erase(id);
		}
		int id() {
			return _id;
		}

		uv_loop_t* loop() {
			return _loop;
		}

		uv_tcp_t& listener() {
			return _socket;
		}

        int conn_count() {
            return (int)_connmap.size();
        }

	private:
		static uint32_t _gid;
		int _id;
		uv_tcp_t _socket;
		uv_loop_t* _loop{nullptr};
		std::unordered_map<uint32_t, std::shared_ptr<TCPConn>> _connmap;
	};
}

#endif
