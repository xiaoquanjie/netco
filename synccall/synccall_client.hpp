#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
#include "coroutine/coroutine.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class ScIo;

class ScClient {
public:
	ScClient();
	~ScClient();
	bool Connect(const std::string& ip, unsigned short port, unsigned int timeout);
	// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply);
	// 0==ok, -1==time out,-2==connect invalid, -3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len);
	bool IsConnected()const;
	void Close();

protected:
	bool _Reconnect();
	void _FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len);
	bool _CheckReply(SocketLib::Buffer* reply);

	ScClient(const ScClient&);
	ScClient& operator=(const ScClient&);

private:
	std::string _ip;
	unsigned short _port;
	unsigned int _timeo;
	unsigned int _packidx;
	netiolib::Buffer _request;
	netiolib::SyncTcpConnector* _socket;
};

inline ScClient::ScClient() {
	_port = 0;
	_timeo = -1;
	_packidx = 0;
	_socket = 0;
}

inline ScClient::~ScClient() {
	Close();
	delete _socket;
}

inline bool ScClient::Connect(const std::string& ip, unsigned short port, unsigned int timeout) {
	if (_socket) {
		return false;
	}
	else {
		_ip = ip;
		_port = port;
		_timeo = timeout;
		return _Reconnect();
	}
}

// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
inline int ScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply) {
	preply = 0;
	if (!_socket) {
		return -2;
	}
	if (!_socket->IsConnected()) {
		if (!_Reconnect())
			return -2;
	}
	_FillRequest(999, msg_type, msg, len);
	if (!_socket->Send(_request.Data(), _request.Length())) {
		Close();
		return -3;
	}
	SocketLib::Buffer* reply = _socket->Recv();
	if (_CheckReply(reply)) {
		preply = reply;
		return 0; // ok
	}
	else {
		Close();
		return -3;
	}
}

// 0==ok, -1==time out,-2==connect invalid, -3 == other error
inline int ScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	if (!_socket) {
		return -2;
	}
	if (!_socket->IsConnected()) {
		if (!_Reconnect())
			return -2;
	}
	_FillRequest(M_ONEWAY_TYPE, msg_type, msg, len);
	if (!_socket->Send(_request.Data(), _request.Length())) {
		Close();
		return -3;
	}
	SocketLib::Buffer* reply = _socket->Recv();
	if (_CheckReply(reply)) {
		return 0; // ok
	}
	else {
		Close();
		return -3;
	}
}

inline bool ScClient::IsConnected()const {
	if (_socket)
		return _socket->IsConnected();
	return false;
}

inline void ScClient::Close() {
	if (_socket) {
		_socket->Close();
	}
}

inline bool ScClient::_Reconnect() {
	delete _socket;
	_socket = new netiolib::SyncTcpConnector;
	if (_socket->Connect(_ip, _port, _timeo)) {
		return true;
	}
	return false;
}

inline void ScClient::_FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	_request.Clear();
	_request.Write(way);
	_request.Write(msg_type);
	_request.Write(++_packidx);
	_request.Write(msg, len);
}

inline bool ScClient::_CheckReply(SocketLib::Buffer* reply) {
	if (reply) {
		unsigned int way_type = 0;
		reply->Read(way_type);
		unsigned int msg_type = 0;
		reply->Read(msg_type);
		unsigned int pack_idx = 0;
		reply->Read(pack_idx);
		if (pack_idx != _packidx)
			return false;
		return true;
	}
	else {
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Э�̰汾
class CoScClient {
	friend class ScServer;
public:
	~CoScClient();
	bool Connect(const std::string& ip, unsigned short port, unsigned int timeout);
	// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply);
	// 0==ok, -1==time out,-2==connect invalid, -3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len);
	bool IsConnected()const;
	void Close();

protected:
	bool _Reconnect();
	void _FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len);
	bool _CheckReply(SocketLib::Buffer* reply);

	CoScClient();
	CoScClient(const CoScClient&);
	CoScClient& operator=(const CoScClient&);

private:
	std::string _ip;
	unsigned short _port;
	unsigned int _timeo;
	unsigned int _packidx;
	netiolib::Buffer _request;
	netiolib::TcpConnectorPtr _socket;
	ScIo* _io;
};

inline CoScClient::CoScClient() {
	_port = 0;
	_timeo = -1;
	_packidx = 0;
	_io = 0;
}

inline CoScClient::~CoScClient() {
	Close();
}

inline bool CoScClient::Connect(const std::string& ip, unsigned short port, unsigned int timeout) {
	if (_socket) {
		return false;
	}
	else {
		_ip = ip;
		_port = port;
		_timeo = timeout;
		return _Reconnect();
	}
}

// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
inline int CoScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply) {
	return -1;
}

// 0==ok, -1==time out,-2==connect invalid, -3 == other error
inline int CoScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	if (!_socket) {
		return -2;
	}
	if (!_socket->IsConnected()) {
		if (!_Reconnect())
			return -2;
	}
	_FillRequest(M_ONEWAY_TYPE, msg_type, msg, len);
	_socket->Send(_request.Data(), _request.Length());
	_CoScInfo_* info = (_CoScInfo_*)_socket->GetSocket().GetData();
	if (!info) {
		Close();
		return -3;
	}
	info->co_id = coroutine::Coroutine::curid();
	coroutine::Coroutine::yield();
	if (!info 
		|| info->error
		|| info->buffer.Length()==0) {
		Close();
		return -3;
	}
	if (_CheckReply(&info->buffer)) {
		return 0; // ok
	}
	else {
		Close();
		return -3;
	}
}

inline bool CoScClient::IsConnected()const {
	if (_socket)
		return _socket->IsConnected();
	return false;
}

inline void CoScClient::Close() {
	if (_socket) {
		_socket->Close();
	}
}

inline bool CoScClient::_Reconnect() {
	_socket.reset(new netiolib::TcpConnector((netiolib::NetIo&)*_io));
	if (_socket->Connect(_ip, _port, _timeo)) {
		_CoScInfo_* info = new _CoScInfo_;
		info->co_id = -1;
		_socket->GetSocket().SetData(info);
		return true;
	}
	return false;
}

inline void CoScClient::_FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len){
	_request.Clear();
	_request.Write(way);
	_request.Write(msg_type);
	_request.Write(++_packidx);
	_request.Write(msg, len);
}

inline bool CoScClient::_CheckReply(SocketLib::Buffer* reply) {
	if (reply) {
		unsigned int way_type = 0;
		reply->Read(way_type);
		unsigned int msg_type = 0;
		reply->Read(msg_type);
		unsigned int pack_idx = 0;
		reply->Read(pack_idx);
		if (pack_idx != _packidx)
			return false;
		return true;
	}
	else {
		return false;
	}
}

M_SYNCCALL_NAMESPACE_END
#endif