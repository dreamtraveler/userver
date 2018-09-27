#include "rpc/ac_login.h"
#include "platform.h"
#include "servlet.h"
#include "log.h"

GX_NS_USING;

struct AC_LoginServlet : Servlet<AC_Login>  {
	int execute(request_type* req, response_type* rsp, NetCoroutine* co) override {
		log_debug("id=%d, name=%d", req->id, req->name);
		rsp->rc = 4 + req->id;
		return 0;
	}
};
