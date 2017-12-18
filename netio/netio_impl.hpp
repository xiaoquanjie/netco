/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/11
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_IMPL_INCLUDE
#define M_NETIO_NETIO_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename NetIoType>
BaseNetIo<NetIoType>::BaseNetIo()
	:_backlog(20) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

template<typename NetIoType>
BaseNetIo<NetIoType>::BaseNetIo(SocketLib::s_uint32_t backlog)
	: _backlog(backlog) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

template<typename NetIoType>
BaseNetIo<NetIoType>::~BaseNetIo() {}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this));
		acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOne(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOne(ep);
}

// ����һ��http����
template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOneHttp(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		HttpSocketPtr clisock(new HttpSocket(*this));
		acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHttpHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOneHttp(ep);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data) {
	try {
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(*this));
		connector->SetData(data);
		connector->AsyncConnect(ep);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOne(ep, data);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep, unsigned int data) {
	try {
		netiolib::HttpConnectorPtr connector(new netiolib::HttpConnector(*this));
		connector->SetData(data);
		connector->AsyncConnect(ep);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOneHttp(ep, data);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::Run() {
	try {
		_ioservice.Run();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("run happend error:"M_ERROR_DESC_STR(error));
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::Stop() {
	try {
		_ioservice.Stop();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("stop happend error:"M_ERROR_DESC_STR(error));
	}
}

template<typename NetIoType>
inline SocketLib::SocketError BaseNetIo<NetIoType>::GetLastError()const {
	return lasterror;
}

template<typename NetIoType>
inline SocketLib::IoService& BaseNetIo<NetIoType>::GetIoService() {
	return _ioservice;
}

template<typename NetIoType>
inline SocketLib::s_uint32_t BaseNetIo<NetIoType>::LocalEndian()const {
	return _endian;
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::_AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	TcpSocketPtr newclisock(new TcpSocket(*this));
	acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::_AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	HttpSocketPtr newclisock(new HttpSocket(*this));
	acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHttpHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
*��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
*��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
*/

// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(const TcpSocketPtr& clisock) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(const TcpConnectorPtr& clisock, SocketLib::SocketError error) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(HttpSocketPtr clisock) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(HttpConnectorPtr clisock, SocketLib::SocketError error) {
}


// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(const TcpSocketPtr& clisock) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(const TcpConnectorPtr& clisock) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(HttpSocketPtr clisock) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(HttpConnectorPtr clisock) {
}

// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(const TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(const TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(HttpSocketPtr clisock, HttpSvrRecvMsg& httpmsg) {
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(HttpConnectorPtr clisock, HttpCliRecvMsg& httpmsg) {
}

M_NETIO_NAMESPACE_END
#endif