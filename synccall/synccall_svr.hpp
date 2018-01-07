#ifndef M_SYNCCALL_SVR_INCLUDE
#define M_SYNCCALL_SVR_INCLUDE

#include "synccall/config.hpp"
#include "synccall/server_handler.hpp"
#include "synccall/synccall_coclient.hpp"
#include "base/thread.hpp"
#include <stdlib.h>
M_SYNCCALL_NAMESPACE_BEGIN

#ifdef M_OPEN_DEBUG_LOG
#define M_PRINT_DEBUG_LOG(info) printf(info)
#else  
#define M_PRINT_DEBUG_LOG(info)
#endif

class ScClient;
class CoScClient;
class ScServer;

struct IScServer {
	friend class ScIo;
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnDisconnected(netiolib::ScConnectorPtr& clisock) = 0;
	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) = 0;
	virtual void OnReceiveData(netiolib::ScConnectorPtr& clisock, netiolib::Buffer& buffer) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ScIo : public netiolib::NetIo {
public:
	ScIo(IScServer* server) {
		_server = server;
	}
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock);
	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock);
	virtual void OnDisconnected(netiolib::ScConnectorPtr& clisock);
	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer);
	virtual void OnReceiveData(netiolib::ScConnectorPtr& clisock, netiolib::Buffer& buffer);
protected:
	IScServer* _server;
};

inline void ScIo::OnConnected(netiolib::TcpSocketPtr& clisock) {
	_server->OnConnected(clisock);
}
inline void ScIo::OnDisconnected(netiolib::TcpSocketPtr& clisock) {
	_server->OnDisconnected(clisock);
}
inline void ScIo::OnDisconnected(netiolib::ScConnectorPtr& clisock) {
	_server->OnDisconnected(clisock);
}
inline void ScIo::OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
	_server->OnReceiveData(clisock, buffer);
}
inline void ScIo::OnReceiveData(netiolib::ScConnectorPtr& clisock, netiolib::Buffer& buffer) {
	_server->OnReceiveData(clisock, buffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ScServer : public IScServer {
	friend class ScIo;
public:
	ScServer();
	~ScServer();
	bool RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler);
	CoScClient* CreateCoScClient();
	void Start(unsigned int thread_cnt);
	void Stop();

protected:
	void Run(void*);
	void OnConnected(netiolib::TcpSocketPtr& clisock);
	void OnDisconnected(netiolib::TcpSocketPtr& clisock);
	void OnDisconnected(netiolib::ScConnectorPtr& clisock);
	void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer);
	void OnReceiveData(netiolib::ScConnectorPtr& clisock, netiolib::Buffer& buffer);
	base::s_uint16_t UniqueId(const std::string& ip, unsigned short port);

private:
	ScIo _io;
	std::list<base::thread*> _threads;
	IServerHandler* _handlers[0xFFFF];
};

inline ScServer::ScServer() 
	:_io(this) {
	memset(_handlers, 0, sizeof(_handlers));
}

inline ScServer::~ScServer() {
	for (std::list<base::thread*>::iterator iter = _threads.begin();
		iter != _threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	_threads.clear();

	for (base::s_uint16_t idx = 0; idx < (sizeof(_handlers) / sizeof(IServerHandler*)); ++idx) {
		delete _handlers[idx];
	}
}

inline bool ScServer::RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler) {
	if (_io.ListenOne(ip, port)) {
		base::s_uint16_t id = UniqueId(ip, port);
		_handlers[id] = handler;
		return true;
	}
	else {
		return false;
	}
}

inline CoScClient* ScServer::CreateCoScClient() {
	CoScClient* client = new CoScClient;
	client->_io = &_io;
	return client;
}

inline void ScServer::Start(unsigned int thread_cnt) {
	if (_threads.empty()) {
		for (unsigned int idx = 0; idx < thread_cnt; ++idx) {
			base::thread* pthread = new base::thread(&ScServer::Run, this, 0);
			_threads.push_back(pthread);
		}
	}
	while (_io.ServiceCount()
		!= _threads.size());
}

inline void ScServer::Stop() {
	_io.Stop();
}

inline void ScServer::Run(void*) {
	printf("%d thread is starting..............\n", base::thread::ctid());
	_io.Run();
	printf("%d thread is leaving..............\n", base::thread::ctid());
}

inline void ScServer::OnConnected(netiolib::TcpSocketPtr& clisock) {
	M_PRINT_DEBUG_LOG("onconnected......\n");
	std::string ip = clisock->LocalEndpoint().Address();
	base::s_uint16_t port = clisock->LocalEndpoint().Port();
	_ScInfo_* pscinfo = new _ScInfo_;
	pscinfo->id = UniqueId(ip, port);
	clisock->GetSocket().SetData(pscinfo);
}

inline void ScServer::OnDisconnected(netiolib::TcpSocketPtr& clisock) {
	M_PRINT_DEBUG_LOG("ondisconnected......\n");
	_ScInfo_* pscinfo = (_ScInfo_*)clisock->GetSocket().GetData();
	delete pscinfo;
}

inline void ScServer::OnDisconnected(netiolib::ScConnectorPtr& clisock) {
	clisock->_mutex.lock();
	int co_id = clisock->co_id;
	int thr_id = clisock->thr_id;
	clisock->co_id = -1;
	clisock->thr_id = 0;
	clisock->valid = false;
	clisock->buffer.Clear();
	clisock->_mutex.unlock();
	if (co_id != -1)
		coroutine::CoroutineTask::addResume(thr_id, co_id);
}

inline void ScServer::OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
	M_PRINT_DEBUG_LOG("onreceivedata......\n");
	_ScInfo_* pscinfo = (_ScInfo_*)clisock->GetSocket().GetData();
	if (!pscinfo) {
		printf("pscinfo is null\n");
		return;
	}
	if (_handlers[pscinfo->id]) {
		unsigned int way_type = 0;
		buffer.Read(way_type);
		unsigned int msg_type = 0;
		buffer.Read(msg_type);
		unsigned int pack_idx = 0;
		buffer.Read(pack_idx);
		pscinfo->buffer.Clear();
		pscinfo->buffer.Write(way_type);
		pscinfo->buffer.Write(msg_type);
		pscinfo->buffer.Write(pack_idx);
		// don't ask why the way_the is 666 or 999
		if (way_type == M_ONEWAY_TYPE) {
			_handlers[pscinfo->id]->OnOneWayDealer(msg_type, buffer);
			clisock->Send(pscinfo->buffer.Data(), pscinfo->buffer.Length());
		}
		else if (way_type == M_TWOWAY_TYPE) {
			_handlers[pscinfo->id]->OnTwoWayDealer(msg_type, buffer, pscinfo->buffer);
			clisock->Send(pscinfo->buffer.Data(), pscinfo->buffer.Length());
		}
		else {
			printf("error way_type(%d)\n", way_type);
		}
	}
}

inline void ScServer::OnReceiveData(netiolib::ScConnectorPtr& clisock, netiolib::Buffer& buffer) {
	int co_id = clisock->co_id;
	int thr_id = clisock->thr_id;
	clisock->co_id = -1;
	clisock->thr_id = 0;
	clisock->buffer.Clear();
	clisock->buffer.Swap(buffer);
	coroutine::CoroutineTask::addResume(thr_id, co_id);
}

inline base::s_uint16_t ScServer::UniqueId(const std::string& ip, unsigned short port) {
	return port;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


M_SYNCCALL_NAMESPACE_END
#endif