
/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/8/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_WIN_IOCP2_INCLUDE
#define M_WIN_IOCP2_INCLUDE

#include "socket/config.hpp"
#include <map>
#include <vector>
#include <algorithm>
#ifdef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

#define M_IOSERVICE_MUTEX_SIZE 128

class IocpService2
{
public:
	class  Access;
	struct Impl;
	struct ImplCloseReq;

	struct IoServiceImpl{
		friend class Access;
		M_SOCKET_DECL IoServiceImpl(IocpService2& service);
		M_SOCKET_DECL IocpService2& GetService();

	private:
		IocpService2&	_service;
		HANDLE			_handler;
		s_uint32_t		_fdcnt;
		MutexLock		_mutex;
		base::slist<ImplCloseReq*> _closereqs;
		base::slist<ImplCloseReq*> _closereqs2;
	};
	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<HANDLE, IoServiceImpl*> IoServiceImplMap;

	struct Oper{
		virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error) = 0;
		virtual void Clear(){}
		virtual ~Oper() {}
	};

	struct Operation : public wsaoverlapped_t{
		M_SOCKET_DECL Operation();
		M_SOCKET_DECL ~Operation();
		s_uint8_t _type;
		Oper*	  _oper;
	};

	template<typename Handler>
	struct AcceptOperation2 : public Oper {
		s_byte_t   _buf[sizeof(sockaddr_storage_t) * 2];
		s_uint32_t _bytes;
		Impl	   _impl;
		Impl	   _accept_impl;
		Handler    _handler;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct ConnectOperation2 : public Oper {
		Handler _handler;
		Impl	_impl;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct WriteOperation2 : public Oper {
		wsabuf_t _wsabuf;
		Handler  _handler;
		Impl	 _impl;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct ReadOperation2 : public Oper {
		wsabuf_t _wsabuf;
		Handler  _handler;
		Impl	 _impl;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
		M_SOCKET_DECL virtual void Clear();
	};

	struct OperationSet{
		Operation _aop;
		Operation _cop;
		Operation _wop;
		Operation _rop;
	};

	template<typename T>
	struct OperationAlloc{
		M_SOCKET_DECL static void AllocOp(Operation& op, s_int32_t type);
	};

	M_SOCKET_DECL IocpService2();

	M_SOCKET_DECL ~IocpService2();

	M_SOCKET_DECL void Run();

	M_SOCKET_DECL void Run(SocketError& error);

	M_SOCKET_DECL void Stop();

	M_SOCKET_DECL void Stop(SocketError& error);

	M_SOCKET_DECL bool Stopped()const;

	M_SOCKET_DECL s_int32_t ServiceCount()const;

protected:
	IocpService2(const IocpService2&);
	IocpService2& operator=(const IocpService2&);

private:
	IoServiceImplVector _implvector;
	IoServiceImplMap	_implmap;
	s_int32_t			_implcnt;
	s_int32_t			_implidx;
	mutable MutexLock	_mutex;
};

M_SOCKET_DECL IocpService2::Operation::Operation() :_oper(0),_type(E_NULL_OP){
}

M_SOCKET_DECL IocpService2::Operation::~Operation(){
	delete _oper;
	_oper = 0;
}

struct IocpService2::Impl{
	friend class Access;
	template<typename T>
	friend struct AcceptOperation2;
	template<typename T>
	friend struct ConnectOperation2;
	template<typename T>
	friend struct WriteOperation2;
	template<typename T>
	friend struct ReadOperation2;

	struct core{
		HANDLE		 _iocp;
		socket_t	 _fd;
		s_uint16_t	 _state;
		OperationSet _op;
		shard_ptr_t<MutexLock> _mutex;
	};

	Impl(){
	}
	void Init() {
		if (!_core) {
			_core.reset(new core);
			_core->_fd = M_INVALID_SOCKET;
			_core->_iocp = 0;
			_core->_state = 0;
		}
	}

private:
	shard_ptr_t<core> _core;
};

struct IocpService2::ImplCloseReq {
	Impl _impl;
	function_t<void()> _handler;
	void Clear() {
		_handler = 0;
		_impl = Impl();
	}
};

class IocpService2::Access
{
public:
	M_SOCKET_DECL static void ConstructImpl(IocpService2& service, Impl& impl, s_uint16_t type);

	M_SOCKET_DECL static void DestroyImpl(IocpService2& service, Impl& impl);

	M_SOCKET_DECL static bool IsOpen(IocpService2& service, Impl& impl, SocketError& error);

	template<typename GettableOptionType>
	M_SOCKET_DECL static void GetOption(IocpService2& service, Impl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL static void SetOption(IocpService2& service, Impl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint RemoteEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint LocalEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Cancel(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void CreateIocp(IocpService2::IoServiceImpl& impl, SocketError& error);

	M_SOCKET_DECL static void DestroyIocp(IocpService2::IoServiceImpl& impl);

	M_SOCKET_DECL static void BindIocp(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void ExecOp(IocpService2& service, IocpService2::Operation* op, s_uint32_t transbyte, bool ok);

	M_SOCKET_DECL static void Run(IocpService2& service, SocketError& error);

	M_SOCKET_DECL static void Stop(IocpService2& service, SocketError& error);

	M_SOCKET_DECL static bool Stopped(const IocpService2& service);

	M_SOCKET_DECL static s_uint32_t GetServiceCount(const IocpService2& service);

	M_SOCKET_DECL static void Close(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Close(IocpService2& service, Impl& impl, function_t<void()> handler, SocketError& error);

	template<typename ProtocolType>
	M_SOCKET_DECL static void Open(IocpService2& service, Impl& impl, const ProtocolType& pt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Bind(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error);
	
	M_SOCKET_DECL static void Listen(IocpService2& service, Impl& impl, s_int32_t flag, SocketError& error);
	
	M_SOCKET_DECL static void Shutdown(IocpService2& service, Impl& impl, EShutdownType what, SocketError& error);
	
	M_SOCKET_DECL static void Accept(IocpService2& service, Impl& impl, Impl& peer, SocketError& error);
	
	M_SOCKET_DECL static void AsyncAccept(IocpService2& service, Impl& accept_impl, Impl& client_impl
		, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error);
	
	template<typename EndPoint>
	M_SOCKET_DECL static void Connect(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec);
	
	template<typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(IocpService2& service, Impl& impl, const EndPoint& ep
		, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error);
	
	M_SOCKET_DECL static s_int32_t RecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);
	
	M_SOCKET_DECL static void AsyncRecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(IocpService2)& hander, SocketError& error);
	
	M_SOCKET_DECL static s_int32_t SendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);
	
	M_SOCKET_DECL static void AsyncSendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(IocpService2)& hander, SocketError& error);

	// -1 == time out,0 == ok,other == error
	M_SOCKET_DECL static s_int32_t Select(Impl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error);

protected:
	M_SOCKET_DECL static void _DoClose(IocpService2::IoServiceImpl* simpl
		, base::slist<ImplCloseReq*>&closereqs, base::slist<ImplCloseReq*>&closereqs2);

	M_SOCKET_DECL static IocpService2::IoServiceImpl* _GetIoServiceImpl(IocpService2& service, Impl& impl);
};

M_SOCKET_DECL IocpService2::IoServiceImpl::IoServiceImpl(IocpService2& service)
	:_service(service),_handler(0),_fdcnt(0){
}

M_SOCKET_DECL IocpService2& IocpService2::IoServiceImpl::GetService(){
	return _service;
}

M_SOCKET_DECL IocpService2::IocpService2()
	:_implcnt(0),_implidx(0){
}

M_SOCKET_DECL IocpService2::~IocpService2(){
	SocketError error;
	Stop(error);
	while (ServiceCount()){
		g_sleep(200);
	}
	ScopedLock scoped(_mutex);
	for (IoServiceImplMap::iterator iter = _implmap.begin(); iter != _implmap.end(); ++iter){
		IoServiceImpl& impl = (IoServiceImpl&)(*iter->second);
		Access::DestroyIocp(impl);
	}
	_implmap.clear();
	_implvector.clear();
}

M_SOCKET_DECL void IocpService2::Run(){
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService2::Run(SocketError& error){
	Access::Run(*this, error);
}

M_SOCKET_DECL void IocpService2::Stop(){
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService2::Stop(SocketError& error){
	Access::Stop(*this, error);
}

M_SOCKET_DECL bool IocpService2::Stopped()const{
	return Access::Stopped(*this);
}

M_SOCKET_DECL s_int32_t IocpService2::ServiceCount()const{
	return Access::GetServiceCount(*this);
}

#include "socket/winsock_init.hpp"
#include "socket/iocp_access.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif
