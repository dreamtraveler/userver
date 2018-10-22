#include "servlet.h"
#include "agentsvr/ac_login.h"
#include "log.h"
#include <iostream>

class AC_LoginServlet : public Servlet<AC_Login> {
	int execute(std::shared_ptr<TCPConn> conn, request_type* req, response_type* rsp, NetCoroutine* co) override {
		log_debug(" execute AC_LoginServlet, id=%d, name=%s, randkey=%d", req->id, req->name.c_str(), req->randKey);
		//std::cout << "\n name.size=" << req->name.size() << "name=" << req->name << std::endl;
		rsp->rc = req->id + 4;
		rsp->name = req->name;
		return 0;
	}
};