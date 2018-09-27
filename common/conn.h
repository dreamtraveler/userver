#ifndef __CONN_H__
#define __CONN_H__

#include "uv.h"
#include "platform.h"

namespace gx {

	typedef enum {
		TCP = 0,
		UDP,
		PIPE
	} stream_type;

	class Conn {
		typedef struct {
			uv_write_t req;
			uv_buf_t buf;
		} write_req_t;


	public:
		Conn();
		~Conn();
	private:
		static int tcp4_echo_start(int port);
		static void on_server_close(uv_handle_t* handle);
		static void on_connection(uv_stream_t* server, int status);

		static void echo_alloc(uv_handle_t* handle,
			size_t suggested_size,
			uv_buf_t* buf);

		static void on_close(uv_handle_t* peer);
		static void after_read(uv_stream_t* handle,
			ssize_t nread,
			const uv_buf_t* buf);

		static void after_shutdown(uv_shutdown_t* req, int status);
		static void after_write(uv_write_t* req, int status);
	private:
		static uv_loop_t* loop;
		static int server_closed;
		static stream_type serverType;
		static uv_tcp_t tcpServer;
		static uv_handle_t* server;
	public:
		int id{ 11 };

	};
}

#endif
