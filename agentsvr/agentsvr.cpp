#include "tcp_conn.h"
#include "tcp_server.h"
#include "reactor.h"
#include "log.h"
#include "timeval.h"
#include "platform.h"
#include "timer.h"
#include "buffer.h"
#include "servlet.h"
#include "impl/ac_login.cpp"
#include "application.h"
#include <memory>
#include <iostream>
#include "object_pool.h"

GX_NS_USING

class A
{
public:
	A() {
		std::cout << "construct A." << std::endl;
	}
	~A() {
		std::cout << "destruct A." << std::endl;
	}
};


//test code
void test_object_pool()
{
	{
		auto pool1 = ObjectPool<AC_LoginReq>::get();
		auto pool2 = ObjectPool<AC_LoginRsp>::get();
	}

}
//---------

void register_needed_servlet() {
	ServletMgr::instance()->register_servlet(new AC_LoginServlet(), true);
}

class Stud {
public:
	Stud() {
		log_debug("construct Stud");
	}
	Stud(const Stud&) {
		log_debug("copy construct Stud");
	}
	Stud& operator=(const Stud&) {
		log_debug("copy assignment Stud");
		return *this;
	}
	Stud(const Stud&&) {
		log_debug("move construct Stud");
	}
	Stud& operator=(const Stud&&) {
		log_debug("move assignment Stud");
		return *this;
	}

	~Stud() {
		log_debug("destruct");
	}
};

void test() {
}

int main() {

	the_app->init();
	register_needed_servlet();

	test_object_pool();
	TCPServer *s1 = new TCPServer(SERVLET_AGENT_CLIENT);
	//TCPServer *s1 = new TCPServer(SERVLET_AGENT);
	bool ok = s1->listen("0.0.0.0", 8890);
	if (!ok) {
		return 0;
	}

	Timer t([&s1]() {
		log_debug("tick on one sec");
		log_debug("coroutine count=%d, conn_count=%d, write_req_count=%d", NetCoroutine::Count(), s1->conn_count(), TCPConn::write_req_count());
	});
	t.start(1000, 3000);

	test();

	the_app->run();
}
