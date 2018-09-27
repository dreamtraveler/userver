#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <string>
#include "platform.h"
#include "buffer.h"

namespace gx {

	struct ISerial {
		virtual ~ISerial() {};
		virtual void serial(Buffer* buf) const = 0;
		virtual void unserial(Buffer* buf) = 0;
		virtual uint16_t msgid() const = 0;
	};

	struct IRpcMessage {
		virtual ~IRpcMessage() {};
		virtual void serial_request(Buffer* buf) const = 0;
		virtual ISerial* get_request() = 0;
		virtual void set_response(Buffer* buf) = 0;
	};
}

#endif
