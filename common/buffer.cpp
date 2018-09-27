#include "buffer.h"

namespace gx {
	const char Buffer::CRLF[] = "\r\n";
	const size_t Buffer::CHEAP_PREPEND_SIZE = 8;
	const size_t Buffer::INITIAL_SIZE = 1024;
}
