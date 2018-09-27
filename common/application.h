#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#define SERVLET_CLIENT 0x0
#define SERVLET_LOGIN 0x1
#define SERVLET_AGENT_CLIENT 0x2
#define SERVLET_AGENT 0x3
#define SERVLET_MAP 0x4
#define SERVLET_MAP_CLIENT 0x5
#define SERVLET_WORLD 0x6
#define SERVLET_TEAM 0x7
#define SERVLET_DB 0x8
#define SERVLET_GAME 0x9
#define SERVLET_UNKNOWN 0xa

#define AC_LOGIN = 0x2001;

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