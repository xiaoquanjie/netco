#include "netio/netio.hpp"
#include "coroutine/coroutine.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

class MyNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "accept one" << endl;
	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {

	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {
		cout << "accept one" << endl;
	}
	virtual void OnConnected(netiolib::HttpConnectorPtr clisock, SocketLib::SocketError error) {

	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {

	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {

	}
	virtual void OnDisconnected(netiolib::HttpSocketPtr clisock) {
		cout << "close one" << endl;
	}
	virtual void OnDisconnected(netiolib::HttpConnectorPtr clisock) {

	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {

	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {

	}
	virtual void OnReceiveData(netiolib::HttpSocketPtr clisock, netiolib::HttpSvrRecvMsg& httpmsg) {
		netiolib::HttpSvrSendMsg& msg = clisock->GetSvrMsg();
		msg.SetBody("newxiaoquanjie", 14);
		clisock->SendHttpMsg();
	}
	virtual void OnReceiveData(netiolib::HttpConnectorPtr clisock, netiolib::HttpCliRecvMsg& httpmsg) {

	}
	void Start(void*) {
		Run();
	}
};

void server() {
	MyNetIo mynetio;
	for (int i = 0; i < 32; ++i) {
		new thread(&MyNetIo::Start, &mynetio, 0);
	}

	thread::sleep(200);
	if (!mynetio.ListenOne("0.0.0.0", 3001)) {
		cout << mynetio.GetLastError().What() << endl;
	}
	else
		cout << "listening....." << endl;

	int i;
	cin >> i;
	mynetio.Stop();
}

void client() {
}

void netlib_test() {
	cout << "select 1 is server,or client :" << endl;
	int i = 0;
	cin >> i;
	if (i == 1)
		server();
	else
		client();
}

void http_server() {
	MyNetIo mynetio;
	for (int i = 0; i < 1; ++i) {
		new thread(&MyNetIo::Start, &mynetio, 0);
	}

	thread::sleep(200);
	if (!mynetio.ListenOneHttp("0.0.0.0", 3002)) {
		cout << mynetio.GetLastError().What() << endl;
	}
	else
		cout << "listening....." << endl;

	int i;
	cin >> i;
	mynetio.Stop();
}

void netlib_http_test() {
	cout << "select 1 is server,or client :" << endl;
	int i = 0;
	cin >> i;
	if (i == 1)
		http_server();
	
}

int main() {

	//netlib_test();
	netlib_http_test();
	return 0;
}