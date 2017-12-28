/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/10
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_INCLUDE
#define M_NETIO_NETIO_INCLUDE

#include "netio/config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;
class TcpConnector;
class HttpSocket;
class HttpConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<HttpSocket>		   HttpSocketPtr;
typedef shard_ptr_t<HttpConnector>	   HttpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > NetIoTcpAcceptorPtr;

#define lasterror base::tlsdata<SocketLib::SocketError,0>::data()

template<typename NetIoType>
class BaseNetIo {
public:
	BaseNetIo();
	BaseNetIo(SocketLib::s_uint32_t backlog);

	virtual ~BaseNetIo();

	// ����һ������
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, SocketLib::s_uint16_t port);

	// ����һ��http����
	bool ListenOneHttp(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port);

	// �첽����
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data = 0);
	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data = 0);

	void ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep, unsigned int data = 0);
	void ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data = 0);

	virtual void Run();
	virtual void Stop();

	// ��ȡ�����쳣
	inline SocketLib::SocketError GetLastError()const;
	inline SocketLib::IoService& GetIoService();
	inline SocketLib::s_uint32_t LocalEndian()const;

	/*
	*����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
	*��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
	*��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
	*/

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(const TcpSocketPtr& clisock);
	virtual void OnConnected(const TcpConnectorPtr& clisock, SocketLib::SocketError error);
	virtual void OnConnected(HttpSocketPtr clisock);
	virtual void OnConnected(HttpConnectorPtr clisock, SocketLib::SocketError error);

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(const TcpSocketPtr& clisock);
	virtual void OnDisconnected(const TcpConnectorPtr& clisock);
	virtual void OnDisconnected(HttpSocketPtr clisock);
	virtual void OnDisconnected(HttpConnectorPtr clisock);

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnReceiveData(const TcpSocketPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(const TcpConnectorPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(HttpSocketPtr clisock, HttpSvrRecvMsg& httpmsg);
	virtual void OnReceiveData(HttpConnectorPtr clisock, HttpCliRecvMsg& httpmsg);


protected:
	void _AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor);
	void _AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor);

protected:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::s_uint32_t  _endian;
};

// class netio
class NetIo : public BaseNetIo<NetIo>
{
public:
	NetIo() :
		BaseNetIo() {
	}
	NetIo(SocketLib::s_uint32_t backlog)
		:BaseNetIo(backlog) {
	}

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);
};

enum {
	E_STATE_STOP = 0,
	E_STATE_START = 1,
	E_STATE_WRITE = 3,
};

template<typename T, typename SocketType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _writerinfo_ {
		base::slist<SocketLib::Buffer*> buffer_pool;
		base::svector<SocketLib::Buffer*> buffer_pool2;
		SocketLib::Buffer*   msgbuffer;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
		~_writerinfo_();
	};

public:
	TcpBaseSocket(BaseNetIo<NetIo>& netio);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// ���������������ζ�����������Ͽ���������е�δ��������ݴ�����ͻ�Ͽ�
	void Close();

	void Send(SocketLib::Buffer*);

	void Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

protected:
	void _WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose(unsigned int state);

	void _Close(unsigned int state);

	bool _TrySendData(bool ignore = false);

protected:
	BaseNetIo<NetIo>& _netio;
	SocketType*  _socket;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	// ״̬��־
	unsigned short _flag;
};

// for stream
template<typename T, typename SocketType>
class TcpStreamSocket : public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		SocketLib::Buffer	  msgbuffer;
		SocketLib::Buffer	  msgbuffer2;
		MessageHeader		  curheader;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	TcpStreamSocket(BaseNetIo<NetIo>& netio);
};

// class tcpsocket
class TcpSocket :
	public TcpStreamSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService> >
{
	friend class BaseNetIo<NetIo>;

public:
	TcpSocket(BaseNetIo<NetIo>& netio)
		:TcpStreamSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

protected:
	void Init() {
		try {
			_remoteep = _socket->RemoteEndPoint();
			_localep = _socket->LocalEndPoint();
			_flag = E_STATE_START;
			_netio.OnConnected(this->shared_from_this());
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}
};

template<typename ConnectorType>
class BaseTcpConnector :
	public TcpStreamSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService> >
{
public:
	BaseTcpConnector(BaseNetIo<NetIo>& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);

protected:
	unsigned int _data;
};

// class tcpconnector
class TcpConnector : public BaseTcpConnector<TcpConnector>
{
public:
	TcpConnector(BaseNetIo<NetIo>& netio)
		:BaseTcpConnector(netio) {
	}
};

// for http
template<typename T, typename SocketType, typename HttpMsgType>
class HttpBaseSocket :
	public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		HttpMsgType httpmsg;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	HttpBaseSocket(BaseNetIo<NetIo>& netio);
};

// class httpsocket
class HttpSocket :
	public HttpBaseSocket<HttpSocket, SocketLib::TcpSocket<SocketLib::IoService>, HttpSvrRecvMsg>
{
	friend class BaseNetIo<NetIo>;
public:
	HttpSocket(BaseNetIo<NetIo>& netio)
		:HttpBaseSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

	HttpSvrSendMsg& GetSvrMsg() {
		return _httpmsg;
	}

	void SendHttpMsg() {
		Send(_httpmsg._pbuffer);
		_httpmsg._flag = 0;
	}

protected:
	void Init() {
		try {
			this->_remoteep = this->_socket->RemoteEndPoint();
			this->_localep = this->_socket->LocalEndPoint();
			this->_flag = E_STATE_START;
			this->_netio.OnConnected(this->shared_from_this());
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}

	HttpSvrSendMsg _httpmsg;
};

template<typename ConnectorType>
class BaseHttpConnector :
	public HttpBaseSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService>
	, HttpCliRecvMsg> {
public:
	BaseHttpConnector(BaseNetIo<NetIo>& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector);

protected:
	unsigned int _data;
};

class HttpConnector : public BaseHttpConnector<HttpConnector>
{
public:
	HttpConnector(BaseNetIo<NetIo>& netio)
		:BaseHttpConnector(netio) {

	}
};


M_NETIO_NAMESPACE_END
#include "netio/netio_impl.hpp"
#include "netio/tsocket_impl.hpp"
#include "netio/tconnector_impl.hpp"
#include "netio/hsocket_impl.hpp"
#include "netio/hconnector_impl.hpp"
#endif