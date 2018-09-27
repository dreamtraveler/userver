#ifndef __REACTOR_H__
#define __REACTOR_H__

#include "platform.h"
#include "uv.h"

namespace gx {
	class Reactor
	{
	private:
		Reactor();
		~Reactor();

		Reactor(const Reactor&) = delete;
		Reactor& operator=(const Reactor&) = delete;

	public:
		static void run();
	private:
		static uv_idle_t idler;
	};
}

#endif
