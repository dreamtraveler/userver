#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <memory>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
#include "platform.h"

namespace gx {

	typedef std::function<void()> EVAction;

	struct QueueNode {
		EVAction data;
		std::unique_ptr<QueueNode> next;
	};


	class EventLoop
	{
	public:
		EventLoop();
		~EventLoop();

		EventLoop(const EventLoop& other) = delete;
		EventLoop& operator=(const EventLoop& other) = delete;

		void update() {
			std::unique_lock <std::mutex> lc(_update_mtx);
			auto dur = std::chrono::milliseconds(1000);
			EVAction act;

			while (!_quit) {
				_cv.wait_for(lc, dur);

				while (!queue_empty()) {
					act = pop_action();
					if (act) {
						act();
					}
				}
			}
		}

		void push_action(EVAction&& act) {
			std::unique_ptr<QueueNode> p = std::make_unique<QueueNode>();
			QueueNode* const new_tail = p.get();

			std::lock_guard<std::mutex> guard(_tail_mtx);
			_tail->data = std::move(act);
			_tail->next = std::move(p);
			_tail = new_tail;
			_cv.notify_one();
		}

		bool queue_empty() {
			return _head.get() == _tail;
		}

		QueueNode* get_tail() {
			std::lock_guard<std::mutex> guard(_tail_mtx);
			return _tail;
		}

		EVAction pop_action() {
			std::lock_guard<std::mutex> guard(_head_mtx);
			if (_head.get() == get_tail()) {
				return nullptr;
			}
			else {
				std::unique_ptr<QueueNode> old_head = std::move(_head);
				_head = std::move(old_head->next);
				return std::move(old_head->data);
			}
		}

		void quit() {
			_quit = true;
		}

	private:
		std::unique_ptr<QueueNode> _head;
		QueueNode* _tail{nullptr};
		std::mutex _head_mtx;
		std::mutex _tail_mtx;
		std::mutex _update_mtx;
		std::condition_variable _cv;
		bool _quit{false};
	};

}

#endif
