#include "event_loop.h"

namespace gx {

	EventLoop::EventLoop() : _head(new QueueNode), _tail(_head.get()), _quit(false)
	{
	}

	EventLoop::~EventLoop()
	{
	}
}
