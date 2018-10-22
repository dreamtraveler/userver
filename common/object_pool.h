#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

#include <functional>
#include <memory>
#include <vector>
#include "platform.h"
#include "serial.h"

namespace gx {

	typedef std::function<void(ISerial*)> DeleterType;
	typedef std::unique_ptr<ISerial, DeleterType> SerialPtr;

	template<typename T>
	class ObjectPool
	{
	public:
		ObjectPool(const ObjectPool&) = delete;
		ObjectPool& operator=(const ObjectPool&) = delete;

		static void recycle_fun(ISerial* t) {
			_pool.emplace_back(SerialPtr(t, recycle_fun));
			log_debug("_pool size = %zd", _pool.size());
		}

		static SerialPtr get() {
			if (_pool.empty()) {
				log_debug("## _pool is empty");
				SerialPtr ptr(new T(), recycle_fun);
				return std::move(ptr);
			}
			else {
				log_debug("## _pool is not empty");
				SerialPtr ptr = std::move(_pool.back());
				_pool.pop_back();
				return std::move(ptr);
			}
		}

		static bool empty() {
			return _pool.empty();
		}

		static size_t size() {
			return _pool.size();
		}

	private:
		static std::vector<SerialPtr> _pool;
	};

	template<typename T>
	std::vector<SerialPtr> ObjectPool<T>::_pool;
}

#endif
