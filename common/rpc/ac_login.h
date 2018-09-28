#ifndef __AC_LOGIN_H__
#define __AC_LOGIN_H__

#include <memory>
#include "serial.h"
#include "log.h"

GX_NS_USING;

struct AC_LoginReq : public ISerial {
	static constexpr const char *the_class_name = "AC_LoginReq";
	//static constexpr int the_message_id = 2;
	static constexpr const char *the_message_name = "AC_Login";

	int32_t id;
	int16_t name;
	AC_LoginReq()
	: id(0),
	  name(0)
	{
		log_debug("AC_LoginReq()");
	}

	~AC_LoginReq() {
		log_debug("~AC_LoginReq()");
	}
	uint16_t msgid() const override {
		return 0x2001;
	}
	void serial(Buffer* buf) const override {
		buf->append_int32(id);
		buf->append_int16(name);
	}

	void unserial(Buffer* buf) override {
		id = buf->read_int32();
		name = buf->read_int16();
	}
};

struct AC_LoginRsp : public ISerial {
	static constexpr const char *the_class_name = "AC_LoginRsp";
	//static constexpr int the_message_id = 2;
	static constexpr const char *the_message_name = "AC_Login";

	int32_t rc;
	AC_LoginRsp()
	: rc(0)
	{
		log_debug("AC_LoginRsp()");
	}
	~AC_LoginRsp() {
		log_debug("~AC_LoginRsp()");
	}

	uint16_t msgid() const override {
		return 0x2001;
	}
	void serial(Buffer* buf) const override {
		buf->append_int32(rc);
	}

	void unserial(Buffer* buf) override {
		rc = buf->read_int32();
	}
};

struct AC_Login : public IRpcMessage {
	typedef AC_LoginReq request_type;
	typedef AC_LoginRsp response_type;
	static constexpr int the_message_id = 0x2001;
	static constexpr const char *the_message_name = "AC_Login";

	void serial_request(Buffer* buf) const override {
		req->serial(buf);
	}

	void set_response(Buffer* buf) override {
		rsp = std::move(std::make_shared<response_type>());
		rsp->unserial(buf);
	}

	ISerial* get_request() override {
		return req.get();
	}

	std::shared_ptr<request_type> req;
	std::shared_ptr<response_type> rsp;
	AC_Login() : req(std::move(std::make_shared<request_type>())) {}
};

#endif
