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

#ifndef M_NETIO_TLS_INCLUDE
#define M_NETIO_TLS_INCLUDE

#include "./config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#include <windows.h>
template<typename T, int N = 0>
class tlsdata
{
public:
	struct _init_
	{
		DWORD _tkey;
		_init_(){
			// ����
			_tkey = TlsAlloc();
			assert(_tkey != TLS_OUT_OF_INDEXES);
		}
		~_init_(){
			TlsFree(_tkey);
		}
	};

	inline static T& data()
	{
		T* pv = 0;
		if (0 == (pv = (T*)TlsGetValue(_data._tkey))){
			pv = new T;
			TlsSetValue(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	tlsdata(const tlsdata&);
	tlsdata& operator=(const tlsdata&);

private:
	static _init_ _data;
};
#else
#include <pthread.h>
template<typename T, int N = 0>
class tlsdata
{
public:
	struct _init_
	{
		pthread_key_t _tkey;
		_init_(){
			// ����
			pthread_key_create(&_tkey, 0);
		}
		~_init_(){
			T* pv = (T*)pthread_getspecific(_tkey);
			pthread_key_delete(_tkey);
			delete pv;
		}
	};

	inline static T& data()
	{
		T* pv = 0;
		if (0 == (pv = (T*)pthread_getspecific(_data._tkey))){
			pv = new T;
			pthread_setspecific(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	tlsdata(const tlsdata&);
	tlsdata& operator=(const tlsdata&);

private:
	static _init_ _data;
};

#endif

template<typename T, int N>
typename tlsdata<T, N>::_init_ tlsdata<T, N>::_data;

M_NETIO_NAMESPACE_END
#endif