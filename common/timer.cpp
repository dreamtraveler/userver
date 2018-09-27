#include "timer.h"
#include "uv.h"
#include "gx.h"

namespace gx {

	Timer::Timer(std::function<void()> cb) :
		_timer(nullptr),
		_timer_handler(cb)
	{
		_timer = (uv_timer_t*)SAFE_MALLOC(sizeof(uv_timer_t));
		int r = uv_timer_init(uv_default_loop(), _timer);
		_timer->data = this;
		return_if_err(r);
	}

	Timer::~Timer() {
		if (_timer) {
			free(_timer);
		}
	}

	void Timer::timer_cb(uv_timer_t *handle) {
		Timer *t = (Timer*)handle->data;
		t->_timer_handler();
	}

	void Timer::start(int timeout, int repeat) {
		int r = uv_timer_start(_timer, Timer::timer_cb, timeout, repeat);
		return_if_err(r);
	}

	void Timer::stop() {
		int r = uv_timer_stop(_timer);
		return_if_err(r);
	}
}
