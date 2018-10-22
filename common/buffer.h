// @see https://github.com/chenshuo/muduo/blob/master/muduo/net/Buffer.h
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <cassert>
#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include "slice.h"
#include "platform.h"

#ifdef GX_PLATFORM_WIN32
#include <WinSock2.h>

#else
#include <arpa/inet.h>
#endif

namespace gx {
	class Buffer {
	public:
		static const size_t CHEAP_PREPEND_SIZE;
		static const size_t INITIAL_SIZE;

		explicit Buffer(size_t initial_size = INITIAL_SIZE, size_t reserved_prepend_size = CHEAP_PREPEND_SIZE)
			: _capacity(reserved_prepend_size + initial_size)
			, _read_index(reserved_prepend_size)
			, _write_index(reserved_prepend_size)
			, _reserved_prepend_size(reserved_prepend_size) {
			_buffer = new char[_capacity];
			assert(length() == 0);
			assert(writable_bytes() == initial_size);
			assert(prependable_bytes() == reserved_prepend_size);
		}

		~Buffer() {
			delete[] _buffer;
			_buffer = nullptr;
			_capacity = 0;
		}

		void swap(Buffer& rhs) {
			std::swap(_buffer, rhs._buffer);
			std::swap(_capacity, rhs._capacity);
			std::swap(_read_index, rhs._read_index);
			std::swap(_write_index, rhs._write_index);
			std::swap(_reserved_prepend_size, rhs._reserved_prepend_size);
		}

		// skip advances the reading index of the buffer
		void skip(size_t len) {
			if (len < length()) {
				_read_index += len;
			}
			else {
				reset();
			}
		}

		// retrieve advances the reading index of the buffer
		// retrieve it the same as skip.
		void retrieve(size_t len) {
			skip(len);
		}

		// truncate discards all but the first n unread bytes from the buffer
		// but continues to use the same allocated storage.
		// It does nothing if n is greater than the length of the buffer.
		void truncate(size_t n) {
			if (n == 0) {
				_read_index = _reserved_prepend_size;
				_write_index = _reserved_prepend_size;
			}
			else if (_write_index > _read_index + n) {
				_write_index = _read_index + n;
			}
		}

		// reset resets the buffer to be empty,
		// but it retains the underlying storage for use by future writes.
		// reset is the same as truncate(0).
		void reset() {
			truncate(0);
		}

		// Increase the capacity of the container to a value that's greater
		// or equal to len. If len is greater than the current capacity(),
		// new storage is allocated, otherwise the method does nothing.
		void reserve(size_t len) {
			if (_capacity >= len + _reserved_prepend_size) {
				return;
			}

			// TODO add the implementation logic here
			grow(len + _reserved_prepend_size);
		}

		// Make sure there is enough memory space to append more data with length len
		void ensure_writable_bytes(size_t len) {
			if (writable_bytes() < len) {
				grow(len);
			}

			assert(writable_bytes() >= len);
		}

		// totext appends char '\0' to buffer to convert the underlying data to a c-style string text.
		// It will not change the length of buffer.
		void totext() {
			append_int8('\0');
			unwrite_bytes(1);
		}

		// TODO XXX Little-Endian/Big-Endian problem.
#define swap_64(x)                          \
    ((((x) & 0xff00000000000000ull) >> 56)       \
     | (((x) & 0x00ff000000000000ull) >> 40)     \
     | (((x) & 0x0000ff0000000000ull) >> 24)     \
     | (((x) & 0x000000ff00000000ull) >> 8)      \
     | (((x) & 0x00000000ff000000ull) << 8)      \
     | (((x) & 0x0000000000ff0000ull) << 24)     \
     | (((x) & 0x000000000000ff00ull) << 40)     \
     | (((x) & 0x00000000000000ffull) << 56))

	// write
	public:
		void write(const void* /*restrict*/ d, size_t len) {
			ensure_writable_bytes(len);
			memcpy(write_begin(), d, len);
			assert(_write_index + len <= _capacity);
			_write_index += len;
		}

		void append(const Slice& str) {
			write(str.data(), str.size());
		}

		void append(const char* /*restrict*/ d, size_t len) {
			write(d, len);
		}

		void append(const void* /*restrict*/ d, size_t len) {
			write(d, len);
		}

		void append_string(std::string s) {
			uint16_t len = (uint16_t)s.size();
			append_int16(len);
			append(s.c_str(), len);
		}

		// append int64_t/int32_t/int16_t with network endian
		void append_int64(int64_t x) {
			int64_t be = swap_64(x);
			write(&be, sizeof be);
		}

		void append_uint64(uint64_t x) {
			uint64_t be = swap_64(x);
			write(&be, sizeof be);
		}

		void append_int32(int32_t x) {
			int32_t be32 = htonl(x);
			write(&be32, sizeof be32);
		}

		void append_uint32(uint32_t x) {
			uint32_t be32 = htonl(x);
			write(&be32, sizeof be32);
		}

		void append_int16(int16_t x) {
			int16_t be16 = htons(x);
			write(&be16, sizeof be16);
		}

		void append_uint16(int16_t x) {
			uint16_t be16 = htons(x);
			write(&be16, sizeof be16);
		}

		void append_int8(int8_t x) {
			write(&x, sizeof x);
		}

		void append_uint8(uint8_t x) {
			write(&x, sizeof x);
		}

		// prepend int64_t/int32_t/int16_t with network endian
		void prepend_int64(int64_t x) {
			int64_t be = swap_64(x);
			prepend(&be, sizeof be);
		}

		void prepend_uint64(uint64_t x) {
			uint64_t be = swap_64(x);
			prepend(&be, sizeof be);
		}

		void prepend_int32(int32_t x) {
			int32_t be32 = htonl(x);
			prepend(&be32, sizeof be32);
		}

		void prepend_uint32(uint32_t x) {
			uint32_t be32 = htonl(x);
			prepend(&be32, sizeof be32);
		}

		void prepend_int16(int16_t x) {
			int16_t be16 = htons(x);
			prepend(&be16, sizeof be16);
		}

		void prepend_uint16(uint16_t x) {
			uint16_t be16 = htons(x);
			prepend(&be16, sizeof be16);
		}

		void prepend_int8(int8_t x) {
			prepend(&x, sizeof x);
		}

		void prepend_uint8(uint8_t x) {
			prepend(&x, sizeof x);
		}

		// Insert content, specified by the parameter, into the front of reading index
		void prepend(const void* /*restrict*/ d, size_t len) {
			assert(len <= prependable_bytes());
			_read_index -= len;
			const char* p = static_cast<const char*>(d);
			memcpy(begin() + _read_index, p, len);
		}

		void unwrite_bytes(size_t n) {
			assert(n <= length());
			_write_index -= n;
		}

		void WriteBytes(size_t n) {
			assert(n <= writable_bytes());
			_write_index += n;
		}

		//Read
	public:
		// Peek int64_t/int32_t/int16_t/int8_t with network endian
		int64_t read_int64() {
			int64_t result = peek_int64();
			skip(sizeof result);
			return result;
		}

		uint64_t read_uint64() {
			uint64_t result = peek_uint64();
			skip(sizeof result);
			return result;
		}

		int32_t read_int32() {
			int32_t result = peek_int32();
			skip(sizeof result);
			return result;
		}

		uint32_t read_uint32() {
			uint32_t result = peek_uint32();
			skip(sizeof result);
			return result;
		}

		int16_t read_int16() {
			int16_t result = peek_int16();
			skip(sizeof result);
			return result;
		}

		uint16_t read_uint16() {
			uint16_t result = peek_uint16();
			skip(sizeof result);
			return result;
		}

		int8_t read_int8() {
			int8_t result = peek_int8();
			skip(sizeof result);
			return result;
		}

		uint8_t read_uint8() {
			uint8_t result = peek_uint8();
			skip(sizeof result);
			return result;
		}

		std::string read_string() {
			uint16_t len = read_uint16();
			return next_string(len);
		}

		Slice toslice() const {
			return Slice(data(), length());
		}

		std::string tostring() const {
			return std::string(data(), length());
		}

		void shrink(size_t reserve) {
			int len = length() + reserve;
			Buffer other(len);
			other.append(toslice());
			swap(other);
		}

		// next returns a slice containing the next n bytes from the buffer,
		// advancing the buffer as if the bytes had been returned by Read.
		// If there are fewer than n bytes in the buffer, next returns the entire buffer.
		// The slice is only valid until the next call to a read or write method.
		Slice next(size_t len) {
			if (len < length()) {
				Slice result(data(), len);
				_read_index += len;
				return result;
			}

			return next_all();
		}

		// next_all returns a slice containing all the unread portion of the buffer,
		// advancing the buffer as if the bytes had been returned by Read.
		Slice next_all() {
			Slice result(data(), length());
			reset();
			return result;
		}

		std::string next_string(size_t len) {
			Slice s = next(len);
			return std::string(s.data(), s.size());
		}

		std::string next_all_string() {
			Slice s = next_all();
			return std::string(s.data(), s.size());
		}

		// read_byte reads and returns the next byte from the buffer.
		// If no byte is available, it returns '\0'.
		char read_byte() {
			assert(length() >= 1);

			if (length() == 0) {
				return '\0';
			}

			return _buffer[_read_index++];
		}

		// UnreadBytes unreads the last n bytes returned
		// by the most recent read operation.
		void unread_bytes(size_t n) {
			assert(n < _read_index);
			_read_index -= n;
		}

		// Peek
	public:
		// Peek int64_t/int32_t/int16_t/int8_t with network endian

		int64_t peek_int64() const {
			assert(length() >= sizeof(int64_t));
			int64_t be64 = 0;
			::memcpy(&be64, data(), sizeof be64);
			return swap_64(be64);
		}

		uint64_t peek_uint64() const {
			assert(length() >= sizeof(uint64_t));
			uint64_t be64 = 0;
			::memcpy(&be64, data(), sizeof be64);
			return swap_64(be64);
		}

		int32_t peek_int32() const {
			assert(length() >= sizeof(int32_t));
			int32_t be32 = 0;
			::memcpy(&be32, data(), sizeof be32);
			return ntohl(be32);
		}

		uint32_t peek_uint32() const {
			assert(length() >= sizeof(uint32_t));
			uint32_t be32 = 0;
			::memcpy(&be32, data(), sizeof be32);
			return ntohl(be32);
		}

		int16_t peek_int16() const {
			assert(length() >= sizeof(int16_t));
			int16_t be16 = 0;
			::memcpy(&be16, data(), sizeof be16);
			return ntohs(be16);
		}

		uint16_t peek_uint16() const {
			assert(length() >= sizeof(uint16_t));
			uint16_t be16 = 0;
			::memcpy(&be16, data(), sizeof be16);
			return ntohs(be16);
		}

		int8_t peek_int8() const {
			assert(length() >= sizeof(int8_t));
			int8_t x = *data();
			return x;
		}

		uint8_t peek_uint8() const {
			assert(length() >= sizeof(uint8_t));
			uint8_t x = *data();
			return x;
		}

	public:
		// data returns a pointer of length Buffer.length() holding the unread portion of the buffer.
		// The data is valid for use only until the next buffer modification (that is,
		// only until the next call to a method like Read, write, reset, or truncate).
		// The data aliases the buffer content at least until the next buffer modification,
		// so immediate changes to the slice will affect the result of future reads.
		const char* data() const {
			return _buffer + _read_index;
		}

		char* write_begin() {
			return begin() + _write_index;
		}

		const char* write_begin() const {
			return begin() + _write_index;
		}

		// length returns the number of bytes of the unread portion of the buffer
		size_t length() const {
			assert(_write_index >= _read_index);
			return _write_index - _read_index;
		}

		// size returns the number of bytes of the unread portion of the buffer.
		// It is the same as length().
		size_t size() const {
			return length();
		}

		// capacity returns the capacity of the buffer's underlying byte slice, that is, the
		// total space allocated for the buffer's data.
		size_t capacity() const {
			return _capacity;
		}

		size_t writable_bytes() const {
			assert(_capacity >= _write_index);
			return _capacity - _write_index;
		}

		size_t prependable_bytes() const {
			return _read_index;
		}
		bool on_starting_point() {
			return _read_index <= _reserved_prepend_size;
		}

		// Helpers
	public:
		const char* find_crlf() const {
			const char* crlf = std::search(data(), write_begin(), CRLF, CRLF + 2);
			return crlf == write_begin() ? nullptr : crlf;
		}

		const char* find_crlf(const char* start) const {
			assert(data() <= start);
			assert(start <= write_begin());
			const char* crlf = std::search(start, write_begin(), CRLF, CRLF + 2);
			return crlf == write_begin() ? nullptr : crlf;
		}

		const char* find_eol() const {
			const void* eol = memchr(data(), '\n', length());
			return static_cast<const char*>(eol);
		}

		const char* find_eol(const char* start) const {
			assert(data() <= start);
			assert(start <= write_begin());
			const void* eol = memchr(start, '\n', write_begin() - start);
			return static_cast<const char*>(eol);
		}
	private:

		char* begin() {
			return _buffer;
		}

		const char* begin() const {
			return _buffer;
		}

		void grow(size_t len) {
			if (writable_bytes() + prependable_bytes() < len + _reserved_prepend_size) {
				//grow the capacity
				size_t n = (_capacity << 1) + len;
				size_t m = length();
				char* d = new char[n];
				memcpy(d + _reserved_prepend_size, begin() + _read_index, m);
				_write_index = m + _reserved_prepend_size;
				_read_index = _reserved_prepend_size;
				_capacity = n;
				delete[] _buffer;
				_buffer = d;
			}
			else {
				// move readable data to the front, make space inside buffer
				assert(_reserved_prepend_size < _read_index);
				size_t readable = length();
				memmove(begin() + _reserved_prepend_size, begin() + _read_index, length());
				_read_index = _reserved_prepend_size;
				_write_index = _read_index + readable;
				assert(readable == length());
				assert(writable_bytes() >= len);
			}
		}

	private:
		char* _buffer{nullptr};
		size_t _capacity{0};
		size_t _read_index{0};
		size_t _write_index{0};
		size_t _reserved_prepend_size{0};
		static const char CRLF[];
	};

}

#endif
