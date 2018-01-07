#include "synccall/synccall.hpp"
#include <iostream>

using namespace std;

struct CDisplayTime
{
	clock_t     miStartTime;
	const char* mpName;
	CDisplayTime(const char* pName) :mpName(pName){
		miStartTime = time(0);//clock();
	}
	~CDisplayTime(){
		if (mpName){
			//std::cout << mpName << " elapsed=" << ((double)(clock() - miStartTime) / CLOCKS_PER_SEC) << std::endl;
			std::cout << mpName << " elapsed=" << ((double)(time(0) - miStartTime)) << std::endl;
		}
	}
};
#define M_DISPLAYTIME() CDisplayTime loDisTime_##__LINE__(__FUNCTION__);

synccall::ScServer server;

class RpcHandler : public synccall::IServerHandler {
public:
	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) {
		std::cout << "error" << std::endl;
	}
	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) {
		if (msg_type == 1) {
			int i = 0;
			request.Read(i);
			reply.Write(i);
		}
		else if (msg_type == 2) {
			std::string name = "null";
			synccall::CoScClient* client = server.CreateCoScClient();
			if (client->Connect("127.0.0.1", 6001,1)) {
				netiolib::Buffer* reply = 0;
				int ret = client->SyncCall(1, 0, 0, reply);
				if (ret == 0) {
					reply->Read(name);
				}
			}
			delete client;
			reply.Write(name);
		}
		else {
			std::cout << "error msg_type=" << msg_type << std::endl;
		}
	}

protected:

};

void synccall_server(){
	server.Start(1);
	server.RegisterHandler("0.0.0.0", 4001, new RpcHandler);
	int i = 0;
	std::cin >> i;
	server.Stop();
}

void synccall_client() {
	int i = 0;
	SocketLib::Buffer request;
	std::string info = "this is a test!!!!";
	for (int i = 0; i < 1024; ++i)
		info.append("a");

	std::string ip;
	std::cout << "input ip:";
	std::cin >> ip;

	synccall::ScClient client;
	//if (!client.Connect("172.16.81.247", 4001,-1)) {
	if (!client.Connect(ip, 4001, -1)) {
		std::cout << "connect fail" << std::endl;
		return;
	}
	std::cout << "connect success" << std::endl;
	
	int idx = 0;
	base::s_uint64_t req_total = 0;
	base::s_uint64_t rep_total = 0;
	{
		M_DISPLAYTIME();
		while (true) {
			//std::cin >> info;
			if (info == "stop")
				break;

			request.Clear();
			req_total += ++idx;
			request.Write(idx);
			SocketLib::Buffer* reply;
			int ret = client.SyncCall(1, request.Data(), request.Length(), reply);
			if (ret != 0) {
				std::cout << "happend error" << std::endl;
				break;
			}
			
			int reply_idx = 0;
			reply->Read(reply_idx);
			rep_total += reply_idx;
			++i;
			if (i >= 100000)
				break;
		}
	}
	std::cout << req_total << " " << rep_total << std::endl;
	std::cin >> i;
}

void synccall_client_getname() {
	std::string ip;
	std::cout << "input ip:";
	std::cin >> ip;

	synccall::ScClient client;
	if (!client.Connect(ip, 4001, -1)) {
		std::cout << "connect fail" << std::endl;
		return;
	}
	std::cout << "connect success" << std::endl;

	SocketLib::Buffer* reply;
	int ret = client.SyncCall(2, 0, 0, reply);
	if (ret != 0) {
		std::cout << "happend error" << std::endl;
	}
	else {
		std::string name;
		reply->Read(name);
		cout << "name:" << name << endl;
	}
}
