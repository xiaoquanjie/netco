#include "synccall/synccall.hpp"
#include <iostream>

using namespace std;

class RpcHandler2 : public synccall::IServerHandler {
public:
	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) {
		std::cout << "error" << std::endl;

	}
	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) {
		if (msg_type == 1) {
			std::string name = "xiaoquanjie";
			reply.Write(name);
		}
		else {
			std::cout << "error msg_type=" << msg_type << std::endl;
		}
	}
};

void synccall_name_server() {
	synccall::ScServer server;
	server.Start(1);
	server.RegisterHandler("0.0.0.0", 6001, new RpcHandler2);
	int i = 0;
	std::cin >> i;
	server.Stop();
}