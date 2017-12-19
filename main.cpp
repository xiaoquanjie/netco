#include "netio/netio.hpp"
#include "coroutine/coroutine.hpp"
#include <iostream>

using namespace std;

class MyNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {

	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {

	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {

	}
	virtual void OnConnected(netiolib::HttpConnectorPtr clisock, SocketLib::SocketError error) {

	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {

	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {

	}
	virtual void OnDisconnected(netiolib::HttpSocketPtr clisock) {

	}
	virtual void OnDisconnected(netiolib::HttpConnectorPtr clisock) {

	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {

	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {

	}
	virtual void OnReceiveData(netiolib::HttpSocketPtr clisock, netiolib::HttpSvrRecvMsg& httpmsg) {

	}
	virtual void OnReceiveData(netiolib::HttpConnectorPtr clisock, netiolib::HttpCliRecvMsg& httpmsg) {

	}
};

int main() {

	coroutine::Coroutine::initEnv();
	/*coroutine::Coroutine::initEnv();
	std::list<int> _list;
	int id = coroutine::Coroutine::create(co_func,0);*/

	return 0;
}