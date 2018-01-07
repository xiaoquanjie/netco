#ifndef M_SYNCCALL_COCLIENT_INCLUDE
#define M_SYNCCALL_COCLIENT_INCLUDE

#include "synccall/config.hpp"
#include "synccall/synccall_client.hpp"
#include "coroutine/coroutine.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class ScIo;

// Э�̰汾
class CoScClient  : public BaseScClient{
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

	CoScClient();
	
	CoScClient(const CoScClient&);
	
	CoScClient& operator=(const CoScClient&);

private:
	netiolib::ScConnectorPtr _socket;
	ScIo* _io;
};

inline CoScClient::CoScClient() {
	_io = 0;
}

inline CoScClient::~CoScClient() {
	Close();
	_io = 0;
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
	preply = 0;
	if (_socket) {
		if (!_socket->IsConnected()) {
			if (!_Reconnect())
				return -2;
		}
		_socket->_mutex.lock();
		if (_socket->valid) {
			_socket->thr_id = base::thread::ctid();
			_socket->co_id = coroutine::Coroutine::curid();
			_socket->_mutex.unlock();
			_FillRequest(M_TWOWAY_TYPE, msg_type, msg, len);
			_socket->Send(_request.Data(), _request.Length());
			coroutine::Coroutine::yield();
			do {
				if (!_CheckReply(&_socket->buffer))
					break;
				preply = &_socket->buffer;
				return 0;
			} while (false);
		}
		else {
			_socket->_mutex.unlock();
		}
		Close();
	}
	return -3;
}

// 0==ok, -1==time out,-2==connect invalid, -3 == other error
inline int CoScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	if (_socket) {
		if (!_socket->IsConnected()) {
			if (!_Reconnect())
				return -2;
		}
		_socket->_mutex.lock();
		if (_socket->valid) {
			_socket->thr_id = base::thread::ctid();
			_socket->co_id = coroutine::Coroutine::curid();
			_socket->_mutex.unlock();
			_FillRequest(M_ONEWAY_TYPE, msg_type, msg, len);
			_socket->Send(_request.Data(), _request.Length());
			coroutine::Coroutine::yield();
			do {
				if (!_CheckReply(&_socket->buffer))
					break;
				return 0;
			} while (false);
		}
		else {
			_socket->_mutex.unlock();
		}
		Close();
	}
	return -3;
}

inline bool CoScClient::IsConnected()const {
	if (_socket)
		return _socket->IsConnected();
	return false;
}

inline void CoScClient::Close() {
	if (_socket) {
		_socket.reset();
	}
}

inline bool CoScClient::_Reconnect() {
	if (!_io) {
		return false;
	}
	else {
		_socket.reset(new netiolib::ScConnector((netiolib::NetIo&)*_io));
		if(_socket->Connect(_ip, _port, _timeo)) {
			_socket->valid = true;
			return true;
		}
		else {
			Close();
			return false;
		}
	}
}



M_SYNCCALL_NAMESPACE_END
#endif