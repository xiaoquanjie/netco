#ifndef M_SYNCCALL_SVR_INCLUDE
#define M_SYNCCALL_SVR_INCLUDE

#include "synccall/config.hpp"
#include "synccall/server_handler.hpp"
#include "synccall/synccall_client.hpp"
#include "base/thread.hpp"
#include <stdlib.h>
M_SYNCCALL_NAMESPACE_BEGIN

#ifdef M_OPEN_DEBUG_LOG
#define M_PRINT_DEBUG_LOG(info) printf(info)
#else  
#define M_PRINT_DEBUG_LOG(info)
#endif

class SyncCallClient;
class SyncCallSvr;

struct ISyncCallSvr {
	friend class SyncCallIo;
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) = 0;
	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) = 0;
	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) = 0;
	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) = 0;
};

class SyncCallIo : public netiolib::NetIo {
public:
	SyncCallIo(ISyncCallSvr* server) {
		_server = server;
	}
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		_server->OnConnected(clisock);
	}
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		_server->OnConnected(clisock, error);
	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		_server->OnDisconnected(clisock);
	}
	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
		_server->OnDisconnected(clisock);
	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
protected:
	ISyncCallSvr* _server;
};

class SyncCallSvr : public ISyncCallSvr {
	friend class SyncCallIo;
public:
	struct scinfo {
		base::s_uint16_t id;
		netiolib::Buffer buffer;
	};

	SyncCallSvr() :_io(this) {
		memset(_handlers, 0, sizeof(_handlers));
	}

	~SyncCallSvr() {
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

	bool RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler) {
		if (_io.ListenOne(ip, port)) {
			base::s_uint16_t id = UniqueId(ip, port);
			_handlers[id] = handler;
			return true;
		}
		else {
			return false;
		}
	}

	void Start(unsigned int thread_cnt) {
		if (_threads.empty()) {
			for (unsigned int idx = 0; idx < thread_cnt; ++idx) {
				base::thread* pthread = new base::thread(&SyncCallSvr::Run, this, 0);
				_threads.push_back(pthread);
			}
		}
		while (_io.ServiceCount() != _threads.size())
			;
	}

	void Stop() {
		_io.Stop();
	}

protected:
	void Run(void*) {
		printf("%d thread is starting..............\n", base::thread::ctid());
		_io.Run();
		printf("%d thread is leaving..............\n", base::thread::ctid());
	}

	void OnConnected(netiolib::TcpSocketPtr& clisock) {
		M_PRINT_DEBUG_LOG("onconnected......\n");
		std::string ip = clisock->LocalEndpoint().Address();
		base::s_uint16_t port = clisock->LocalEndpoint().Port();
		scinfo* pscinfo = new scinfo;
		pscinfo->id = UniqueId(ip, port);
		clisock->GetSocket().SetData(pscinfo);
	}
	void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		
	}

	void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		M_PRINT_DEBUG_LOG("ondisconnected......\n");
		scinfo* pscinfo = (scinfo*)clisock->GetSocket().GetData();
		delete pscinfo;
	}
	void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
	}

	void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		M_PRINT_DEBUG_LOG("onreceivedata......\n");
		scinfo* pscinfo = (scinfo*)clisock->GetSocket().GetData();
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
	void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		
	}

	base::s_uint16_t UniqueId(const std::string& ip, unsigned short port) {
		return port;
	}

private:
	SyncCallIo _io;
	std::list<base::thread*> _threads;
	IServerHandler* _handlers[0xFFFF];
};


M_SYNCCALL_NAMESPACE_END
#endif