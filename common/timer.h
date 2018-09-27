#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include "uv.h"
#include "platform.h"

namespace gx {

	class Timer
	{
	public:
		Timer(std::function<void()> cb);
		~Timer();

		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;

		void start(int timeout, int repeat = 0);
		void stop();
		static void timer_cb(uv_timer_t *handle);

	private:
		uv_timer_t * _timer{nullptr};
		std::function<void()> _timer_handler;
	};
}

#endif
