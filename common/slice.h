#ifndef __SLICE_H__
#define __SLICE_H__

#include <string.h>
#include <assert.h>
#include <string>

namespace gx {

	// Copy from leveldb project
	// @see https://github.com/google/leveldb/blob/master/include/leveldb/slice.h

	class Slice {
	public:
		typedef char value_type;

	public:
		// Create an empty slice.
		Slice() : _data(""), _size(0) {}

		// Create a slice that refers to d[0,n-1].
		Slice(const char* d, size_t n) : _data(d), _size(n) {}

		// Create a slice that refers to the contents of "s"
		Slice(const std::string& s) : _data(s.data()), _size(s.size()) {}

		// Create a slice that refers to s[0,strlen(s)-1]
		Slice(const char* s) : _data(s), _size(strlen(s)) {}

		// Return a pointer to the beginning of the referenced data
		const char* data() const {
			return _data;
		}

		// Return the length (in bytes) of the referenced data
		size_t size() const {
			return _size;
		}

		// Return true if the length of the referenced data is zero
		bool empty() const {
			return _size == 0;
		}

		// Return the ith byte in the referenced data.
		// REQUIRES: n < size()
		char operator[](size_t n) const {
			assert(n < size());
			return _data[n];
		}

		// Change this slice to refer to an empty array
		void clear() {
			_data = "";
			_size = 0;
		}

		// Drop the first "n" bytes from this slice.
		void remove_prefix(size_t n) {
			assert(n <= size());
			_data += n;
			_size -= n;
		}

		// Return a string that contains the copy of the referenced data.
		std::string tostring() const {
			return std::string(_data, _size);
		}

		// Three-way comparison.  Returns value:
		//   <  0 if "*this" <  "b",
		//   == 0 if "*this" == "b",
		//   >  0 if "*this" >  "b"
		int compare(const Slice& b) const;

	private:
		const char* _data{nullptr};
		size_t _size{0};
	};

	typedef Slice slice;

	//---------------------------------------------------------
	//typedef Map<Slice, Slice> SliceSliceMap;
	//---------------------------------------------------------

	inline bool operator==(const Slice& x, const Slice& y) {
		return ((x.size() == y.size()) &&
			(memcmp(x.data(), y.data(), x.size()) == 0));
	}

	inline bool operator!=(const Slice& x, const Slice& y) {
		return !(x == y);
	}

	inline bool operator<(const Slice& x, const Slice& y) {
		return x.compare(y) < 0;
	}


	inline int Slice::compare(const Slice& b) const {
		const size_t min_len = (_size < b._size) ? _size : b._size;
		int r = memcmp(_data, b._data, min_len);

		if (r == 0) {
			if (_size < b._size) {
				r = -1;
			}
			else if (_size > b._size) {
				r = +1;
			}
		}

		return r;
	}
}

#endif
