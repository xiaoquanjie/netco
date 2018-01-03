#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallClient {
	friend class SyncCallSvr;
public:
	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(const int msg_type, const netiolib::Buffer& request, netiolib::Buffer& reply) {
		


		return 0;
	}

	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(const int msg_type, const char* msg,SocketLib::s_uint32_t len) {
		if (_connector) {
			if (!_connector->IsConnected()) {
				//_connector.reset()
			}
		}
		else {
			return -2;
		}

		netiolib::Buffer request;
		request.Write(666);
		request.Write(msg_type);
		request.Write(msg, len);

		return 0;
	}

	bool IsConnected()const {
		if (_connector)
			return _connector->IsConnected();
		return false;
	}

private:
	netiolib::TcpConnectorPtr _connector;
};

M_SYNCCALL_NAMESPACE_END
#endif