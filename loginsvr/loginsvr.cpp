#include "tcp_conn.h"
#include "tcp_server.h"
#include "reactor.h"
#include "log.h"
#include "timeval.h"
#include "platform.h"
#include "timer.h"
#include "buffer.h"
#include "servlet.h"
#include "rpc/ac_login.h"
#include "application.h"

GX_NS_USING

void register_needed_servlet() {
	//ServletMgr::instance()->register_servlet(new AC_LoginServlet(), false);
}

std::vector<std::shared_ptr<TCPConn>> conn_vec;
static int __id;
int MAX_CONN = 5000;
void test(co_pull_t &sink) {
	co_param_t param  = sink.get();
	uint32_t id = std::get<0>(param);
	auto co = NetCoroutine::find(id);
	co->sink(&sink);

	int len = (int)conn_vec.size() < MAX_CONN ? conn_vec.size() : MAX_CONN;
	auto tconn = conn_vec[id % len];
	if (len == 0) {
		co->finished(true);
        return;
    }
	if (!tconn->is_connect()) {
		log_debug("is_connect=false");
		co->finished(true);
		return;
	}

	bool ok;
	do {
		AC_Login msg;
		msg.req->id = id;
		msg.req->name = 3;
		log_debug("1 before call, id=%d", msg.req->id);
		ok = co->call(tconn.get(), msg);
		if (!ok) {
			return;
		}
		log_debug("1 after call, rc=%d", msg.rsp->rc);
	} while (false);

	do {
		AC_Login msg;
		msg.req->id = id;
		msg.req->name = 3;
		log_debug("2 before call, id=%d", msg.req->id);
		ok = co->call(tconn.get(), msg);
		if (!ok) {
			return;
		}
		log_debug("2 after call, rc=%d", msg.rsp->rc);
	} while (false);

	co->finished(true);
}

static int connected_count = 0;

void build_connect() {
	auto conn = std::make_shared<TCPConn>();
	conn->connect_ok_handler([]() {
		connected_count++;
	});
	conn->connect_fail_handler([]() {
		//connected_count--;
	});
	int cid = conn->id();
	conn->disconnect_handler([cid]() {
		auto it = conn_vec.begin();
		log_debug("disconnect_handler, conn.id=%zd", cid);
		for (; it != conn_vec.end(); ++it) {
			if ((*it)->id() == cid) {
				conn_vec.erase(it);
				connected_count--;
				if (connected_count == 0) {
					conn_vec.resize(0);
				}
				return;
			}
		}
	});
	conn_vec.emplace_back(conn);
	conn->connect("127.0.0.1", 8890);
}

int main() {
	the_app->init();

	for (int i = 0; i < MAX_CONN; ++i) {
		build_connect();
	}

	Timer t([]() {
		for (int i = 0; i < MAX_CONN; ++i) {
			auto cofun = std::bind(test,  _1);
			NetCoroutine::spawn(cofun);
		}
	});

	Timer t2([]() {
		log_debug("************ coroutine count=%d, connected_count=%d", NetCoroutine::Count(), connected_count);
	});

	Timer t3([&t]() {
		t.stop();

		auto it = conn_vec.begin();
		for (; it != conn_vec.end(); ++it) {
			(*it)->close();
		}
	});
	
	t.start(2000, 30000);
	t2.start(2000, 1000);

	t3.start(60000, 0);

	the_app->run();

	log_debug("app over.");
}
