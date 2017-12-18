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

#ifndef M_SERVICE_INCLUDE
#define M_SERVICE_INCLUDE

#include "win_iocp.hpp"
#include "linux_epoll.hpp"
#include "win_iocp2.hpp"
M_SOCKET_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
typedef IocpService		IoService2; // 不建议使用这个
typedef IocpService2	IoService;
#else
typedef EpollService	IoService;
typedef EpollService	IoService2;
#endif





M_SOCKET_NAMESPACE_END
#endif