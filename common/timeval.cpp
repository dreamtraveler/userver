#include "timeval.h"

namespace gx {

	volatile timeval_t the_now = gettimeofday();
	timeval_t the_logic_offset = 0;
	long the_timezone = 0;

	std::string strtime(timeval_t time) noexcept {
		time_t t = time / 1000;
		struct tm tm;
#ifdef _WIN32
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif
		char buf[128];
#ifdef _WIN32
		sprintf_s(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
#else
		sprintf(buf, "%04u-%02u-%02u %02u:%02u:%02u",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif
		return buf;
	}
}

