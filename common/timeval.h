#ifndef __GX_TIMEVAL_H__
#define __GX_TIMEVAL_H__

#include <string>
#include <ctime>
#include <chrono>
#include "platform.h"

namespace gx {

	typedef uint64_t timeval_t;
	extern volatile timeval_t the_now;
	extern timeval_t the_logic_offset;
	extern long the_timezone;

	inline long init_timezone() noexcept {
		//if (the_timezone == 0) {
		//	_get_timezone(&the_timezone);
		//}
		return the_timezone;
	}

	inline timeval_t gettimeofday() noexcept {
		return the_now;
	}

	inline timeval_t logic_time() noexcept {
		return the_now + the_logic_offset;
	}

	inline timeval_t adjust_time() noexcept {
		the_now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
			).count();
		return the_now;
	}

	inline timeval_t the_second(timeval_t t = 0) noexcept {
		return (t ? t : logic_time()) / 1000;
	}

	inline timeval_t the_minute(timeval_t t = 0) noexcept {
		return (t ? t : logic_time()) / (1000 * 60);
	}

	inline timeval_t the_hour(timeval_t t = 0) noexcept {
		return (t ? t : logic_time()) / (1000 * 60 * 60);
	}

	inline timeval_t the_day(timeval_t t = 0) noexcept {
		return ((t ? t : logic_time()) + 1000 * 60 * 60 * 8) / (1000 * 60 * 60 * 24);
	}

	inline timeval_t the_week(timeval_t t = 0) noexcept {
		return (t ? t : logic_time()) / (1000 * 60 * 60 * 24 * 7);
	}

	std::string strtime(timeval_t time) noexcept;

	struct date_t {
		date_t() noexcept
			: msec(),
			second(),
			minute(),
			hour(),
			day(),
			week()
		{ }

		date_t(timeval_t t) noexcept {
			*this = t;
		}

		timeval_t msec;
		timeval_t second;
		unsigned minute;
		unsigned hour;
		unsigned day;
		unsigned week;
		unsigned month;
		unsigned year;

		timeval_t operator=(timeval_t t) noexcept {
			msec = t;
			second = t / 1000;
			minute = (unsigned)(second / 60);
			hour = minute / 60;
			day = (hour + the_timezone) / 24;
			week = day / 7;
			return t;
		}
	};
}

#endif

