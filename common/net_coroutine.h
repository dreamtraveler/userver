#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "boost/coroutine2/all.hpp"
#include "log.h"

namespace gx {
	class Buffer;
	class ServletBase;
	class TCPConn;
	struct ISerial;
	struct IRpcMessage;

	typedef std::tuple<uint32_t, Buffer*> co_param_t;
	typedef boost::coroutines2::coroutine<co_param_t>::push_type co_push_t;
	typedef boost::coroutines2::coroutine<co_param_t>::pull_type co_pull_t;
	typedef std::function<void(co_pull_t &sink)> co_fun_t;

	class NetCoroutine : public std::enable_shared_from_this<NetCoroutine>
	{
	public:
		NetCoroutine(uint32_t seq, co_fun_t cofun, std::shared_ptr<TCPConn> conn, ServletBase* servlet)
			: _seq(seq), _conn(conn), _servlet(servlet), _source(cofun),_sink(nullptr)
		{
			_id = _incrid++;
		}

		NetCoroutine(co_fun_t cofun)
			: _seq(0), _conn(nullptr), _servlet(nullptr), _source(std::move(cofun))
		{
			_id = _incrid++;
			_seq = _id;
		}

		~NetCoroutine() {
			log_debug("################### ~NetCoroutine()");
		}

		uint32_t id() {
			return _id;
		}

		void resume(co_param_t& param) {
			_source(param);
		}
		void yield() {
			(*_sink)();
		}

		void sink(co_pull_t* sink) {
			_sink = sink;
		}

		std::shared_ptr<TCPConn> conn() {
			return _conn;
		}

		ServletBase* servlet() {
			return _servlet;
		}
		uint32_t seq() {
			return _seq;
		}

		void finished(bool b) {
			_finished = b;
		}
		bool finished() const {
			return _finished;
		}

		static NetCoroutine* find(uint32_t id) {
			auto it = _co_map.find(id);
			if (it != _co_map.end()) {
				return it->second.get();
			}
			else {
				return nullptr;
			}
		}
		static void remove(uint32_t id) {
			_co_map.erase(id);
		}
		static NetCoroutine* spawn(int32_t seq, co_fun_t cofun, std::shared_ptr<TCPConn> conn, ServletBase* servlet) {
			std::shared_ptr<NetCoroutine> co = std::make_shared<NetCoroutine>(seq, cofun, conn, servlet);
			_co_map.emplace(co->id(), co);
			return co.get();
		}

		static void spawn(co_fun_t cofun) {
			std::shared_ptr<NetCoroutine> co = std::make_shared<NetCoroutine>(std::move(cofun));
			_co_map.emplace(co->id(), co);
			co_param_t param(co->id(), nullptr);
			co->resume(param);
			if (co->finished()) {
				_co_map.erase(co->id());
			}
		}

		static void resume_all() {
			std::vector<std::shared_ptr<NetCoroutine>> vec;
			auto it = _co_map.begin();
			for (; it != _co_map.end(); ++it) {
				vec.emplace_back(it->second);
			}

			auto vecit = vec.begin();
			for (; vecit != vec.end(); ++vecit) {
				co_param_t param((*vecit)->id(), nullptr);
				(*vecit)->resume(param);
			}
			vec.clear();
		}

		bool call(TCPConn* conn, IRpcMessage& msg);

		static int Count() {
			return _co_map.size();
		}

	private:
		static std::unordered_map<unsigned, std::shared_ptr<NetCoroutine>> _co_map;
		static uint32_t _incrid;
		uint32_t _id;
		uint32_t _seq;
		std::shared_ptr<TCPConn> _conn;
		ServletBase* _servlet;
		//TODO Ö»´æ push
		co_push_t _source;
		co_pull_t* _sink;
		bool _finished{false};
	};
}

#endif
