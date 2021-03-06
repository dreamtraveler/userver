#ifndef __SERVLET_H__
#define __SERVLET_H__

#include <string>
#include <unordered_map>
#include <memory>
#include <tuple>
#include "serial.h"
#include "net_coroutine.h"
#include "object_pool.h"

namespace gx {
	class TCPConn;

	class ServletBase {
		friend class ServletMgr;
	public:
		ServletBase(uint32_t id, const char* name) noexcept : _id(id), _name(name), _use_coroutine(false) { }

		ServletBase(const ServletBase&) = delete;
		ServletBase& operator=(const ServletBase&) = delete;


	public:
		uint32_t id() {
			return _id;
		}
		const char* name() const noexcept {
			return _name;
		}
		bool use_coroutine() {
			return _use_coroutine;
		}

		virtual SerialPtr create_request(Buffer* stream) = 0;
		virtual SerialPtr create_response() = 0;

		virtual int sync_execute(std::shared_ptr<TCPConn> conn, ISerial* req, ISerial* rsp) = 0;
		virtual int async_execute(std::shared_ptr<TCPConn> conn, ISerial* req, ISerial* rsp, NetCoroutine* co) = 0;

	private:
		uint32_t _id;
		bool _use_coroutine{false};
		const char* _name{nullptr};
	};

	template<typename _T, typename _Request = typename _T::request_type, typename _Response = typename _T::response_type>
	class Servlet : public ServletBase {
	public:
		typedef _Request request_type;
		typedef _Response response_type;
		typedef _T message_type;
	public:

		Servlet() noexcept : ServletBase(_T::the_message_id, _T::the_message_name) {}

		SerialPtr create_request(Buffer* buf) override {
			auto req = ObjectPool<request_type>::get();
			req->unserial(buf);
			return std::move(req);
		}

		SerialPtr create_response() override {
			auto rsp = ObjectPool<response_type>::get();
			return std::move(rsp);
		}

		int sync_execute(std::shared_ptr<TCPConn> conn, ISerial* req, ISerial* rsp) override {
			return execute(std::move(conn), static_cast<request_type*>(req), static_cast<response_type*>(rsp));
		}

		int async_execute(std::shared_ptr<TCPConn> conn, ISerial* req, ISerial* rsp, NetCoroutine* co) override {
			return execute(std::move(conn), static_cast<request_type*>(req), static_cast<response_type*>(rsp), co);
		}

		int virtual execute(std::shared_ptr<TCPConn> conn, request_type* req, response_type* rsp) { return 0; }
		int virtual execute(std::shared_ptr<TCPConn> conn, request_type* req, response_type* rsp, NetCoroutine* co) { return 0; }
	};

	class ServletMgr {
	public:
		~ServletMgr();
		static ServletMgr *instance() {
			static ServletMgr *o = nullptr;
			if (o == nullptr) {
				o = new ServletMgr();
			}
			return o;
		}

		ServletMgr(const ServletMgr&) = delete;
		ServletMgr& operator=(const ServletMgr&) = delete;

		void register_servlet(ServletBase* servlet, bool use_coroutine) {
			servlet->_use_coroutine = use_coroutine;
			if (!_servlet_map.emplace(servlet->id(), servlet).second) {
				log_fatal("repeatly register servlet '%s(%x)'.", servlet->name(), servlet->id());
			}
		}

		void co_execute(co_pull_t &sink);
		int execute(std::shared_ptr<TCPConn> conn, uint32_t seq, uint16_t msgid, Buffer* buf);
		int execute_response(uint32_t seq, uint16_t msgid, Buffer* buf);

	private:
		ServletMgr();
		std::unordered_map<unsigned, ServletBase*> _servlet_map;
		std::function<void(co_pull_t &sink)> _cofun{nullptr};
	};
}

#endif