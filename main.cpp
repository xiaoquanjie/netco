#include "netio/netio.hpp"
#include <iostream>

using namespace std;

class MyNetIo : public netiolib::NetIo {
public:
	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {

	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {

	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {

	}
	virtual void OnConnected(netiolib::HttpConnectorPtr clisock, SocketLib::SocketError error) {

	}

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {

	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {

	}
	virtual void OnDisconnected(netiolib::HttpSocketPtr clisock) {

	}
	virtual void OnDisconnected(netiolib::HttpConnectorPtr clisock) {

	}

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
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

	return 0;
}