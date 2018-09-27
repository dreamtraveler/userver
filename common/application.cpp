#include "event_loop.h"
#include "application.h"
#include "log.h"
#include "timeval.h"
#include "reactor.h"

namespace gx {

	const std::unique_ptr<Application> the_app = std::make_unique<Application>();
	Application::Application() : _id(-1) {
		_mainloop.reset(new EventLoop());
	}

	Application::~Application()
	{
	}

	bool Application::init() {
		adjust_time();
		the_log_printer->init();
		return true;
	}

	void Application::run() {
		Reactor::run();
	}
}