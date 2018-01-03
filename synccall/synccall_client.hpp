#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallClient {
public:
	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(const int msg_type, const netiolib::Buffer& request, netiolib::Buffer& reply) {
		

		return 0;
	}

	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(const int msg_type, const netiolib::Buffer& request) {
		return 0;
	}

	bool IsConnected()const {
		if (_connector)
			return _connector->IsConnected();
		return false;
	}

	bool Connect() {

	}

	void SetTimeOut() {

	}

private:
	netiolib::TcpConnectorPtr _connector;
};

M_SYNCCALL_NAMESPACE_END
#endif