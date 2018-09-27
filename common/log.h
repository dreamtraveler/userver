#ifndef __GX_LOG_H__
#define __GX_LOG_H__

#include <cstdarg>
#include "platform.h"
#include <thread>
#include <memory>
#include "event_loop.h"
#include "platform.h"

namespace gx {

	class EventLoop;

	enum {
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
		LOG_FATAL,
	};


	class LogPrinter {
	public:
		//LogPrinter(const LogPrinter&) = delete;
		LogPrinter& operator=(const LogPrinter&) = delete;

		void init();
		virtual void vprintf(int level, const char *file, size_t line, const char *fmt, va_list ap) noexcept;
	private:
		void consumer();
	private:
		EventLoop * _loop;
		std::thread _t;
	};

	extern std::shared_ptr<LogPrinter> the_log_printer;

	inline void log(int level, const char *file, size_t line, const char *fmt, ...) noexcept GX_PRINTF_ATTR(4, 5);
	inline void log(int level, const char *file, size_t line, const char *fmt, ...) noexcept {
		va_list ap;
		va_start(ap, fmt);
		the_log_printer->vprintf(level, file, line, fmt, ap);
	}

	void print_back_trace() noexcept;

}

#define gx_log(level, fmt, ...) gx::log(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)     gx_log(gx::LOG_DEBUG,   fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)      gx_log(gx::LOG_INFO,    fmt, ##__VA_ARGS__)
#define log_warning(fmt, ...)   gx_log(gx::LOG_WARNING, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)     gx_log(gx::LOG_ERROR,   fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)     gx_log(gx::LOG_FATAL,   fmt, ##__VA_ARGS__)

#endif
