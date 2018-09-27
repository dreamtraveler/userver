#include "reactor.h"
#include "log.h"
#include "timeval.h"
#include "timer.h"

namespace gx {
	uv_idle_t Reactor::idler;
	Reactor::Reactor()
	{
	}
	Reactor::~Reactor()
	{
	}

	void Reactor::run() {
		Timer adjust_time_timer(adjust_time);
		adjust_time_timer.start(0, 100);

		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	}
}
