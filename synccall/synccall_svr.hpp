#ifndef M_SYNCCALL_SVR_INCLUDE
#define M_SYNCCALL_SVR_INCLUDE

#include "synccall/config.hpp"
#include "synccall/server_handler.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallClient;

class SyncCallSvr : public netiolib::NetIo {
public:
	bool RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler);

	SyncCallClient* CreateClient();

	void Start(unsigned int thread_cnt);

	void Stop();

protected:
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {
	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
	}

	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
	}

	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
	}

private:
	typedef std::map<int, IServerHandler*> SvrHandlerMap;
	SvrHandlerMap _svrhandler_map;
};


inline bool SyncCallSvr::RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler) {

}

M_SYNCCALL_NAMESPACE_END
#endif