#include "netio/netio.hpp"
#include "coroutine/coroutine.hpp"
#include <iostream>
#include "base/thread.hpp"
#include "synccall/synccall.hpp"

using namespace std;

class MyNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "OnConnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "connect fail :" << error.What() << endl;
		}
		else {
			cout << "connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
		}
	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {
		cout << "OnConnected one http : " << clisock->RemoteEndpoint().Address()
			<< " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnConnected(netiolib::HttpConnectorPtr clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "http connect fail :" << error.What() << endl;
		}
		else {
			cout << "http connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
		}
	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(netiolib::HttpSocketPtr clisock) {
		cout << "OnDisconnected one http : " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(netiolib::HttpConnectorPtr clisock) {
		cout << "OnDisconnected one http connector: " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		
	}
	virtual void OnReceiveData(netiolib::HttpSocketPtr clisock, netiolib::HttpSvrRecvMsg& httpmsg) {
		//cout << httpmsg.GetRequestLine() << endl;
		netiolib::HttpSvrSendMsg& msg = clisock->GetSvrMsg();
		msg.SetBody("newxiaoquanjie", 14);
		clisock->SendHttpMsg();
	}
	virtual void OnReceiveData(netiolib::HttpConnectorPtr, netiolib::HttpCliRecvMsg& httmsg) {
		cout << httmsg.GetRespondLine() << endl;
		cout << httmsg.GetHeader() << endl;
		cout << httmsg.GetBody() << endl;
	}

	void Start(void*) {
		Run();
	}
};


void server() {
	MyNetIo mynetio;
	for (int i = 0; i < 32; ++i) {
		new base::thread(&MyNetIo::Start, &mynetio, 0);
	}

	base::thread::sleep(200);
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
	MyNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new base::thread(&MyNetIo::Start, &test_io, 0);
	}

	base::thread::sleep(200);
	if (!test_io.ListenOneHttp("0.0.0.0", 3002)) {
		cout << test_io.GetLastError().What() << endl;
	}
	else
		cout << "listening....." << endl;

	int i;
	cin >> i;
	test_io.Stop();
}

void netlib_http_test() {
	cout << "select 1 is server,or client :" << endl;
	int i = 0;
	cin >> i;
	if (i == 1)
		http_server();
	
}


int main() {

	//synccall::SyncCallSvr::

	//netlib_test();
	//netlib_http_test();
	return 0;
}