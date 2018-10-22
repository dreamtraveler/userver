#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include <memory>

namespace gx {
	class EventLoop;

	class Application
	{
	public:
		Application();
		~Application();
		EventLoop* loop() const {
			return _mainloop.get();
		}
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		bool init();
		void run();
		int id() const {
			return _id;
		}

	private:
		std::shared_ptr<EventLoop> _mainloop;
		int _id;
	};

	extern const std::unique_ptr<Application> the_app;
}

#endif