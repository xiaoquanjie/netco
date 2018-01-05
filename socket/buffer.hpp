#ifndef M_BUFFER_INCLUDE
#define M_BUFFER_INCLUDE

#include "socket/config.hpp"
M_SOCKET_NAMESPACE_BEGIN
#define M_BUFFER_DEFAILT_SIZE 1024

class Buffer
{
public:
	struct _data_ {
		s_uint32_t	_size;
		s_uint32_t	_pos;
		s_uint32_t	_offset;
		s_byte_t*	_data;
	};

	M_SOCKET_DECL Buffer();

	M_SOCKET_DECL Buffer(s_uint32_t hdrlen);

	M_SOCKET_DECL ~Buffer();

	M_SOCKET_DECL void Clear();

	M_SOCKET_DECL void RemoveData(s_uint32_t len);

	M_SOCKET_DECL s_byte_t* Data();

	M_SOCKET_DECL const s_byte_t* Data()const;

	M_SOCKET_DECL s_byte_t* Raw();

	M_SOCKET_DECL const s_byte_t* Raw()const;

	M_SOCKET_DECL s_uint32_t Capacity()const;

	M_SOCKET_DECL s_uint32_t Size()const;

	M_SOCKET_DECL s_uint32_t Length()const;

	M_SOCKET_DECL void Write(const void* data, s_uint32_t len);

	template<typename T>
	M_SOCKET_DECL void Write(T const& value);

	M_SOCKET_DECL void Write(std::string const& value);

	template<typename T>
	Buffer& operator<<(T const& value){
		Write(value);
		return *this;
	}

	M_SOCKET_DECL void Read(void* data, s_uint32_t len);

	template<typename T>
	M_SOCKET_DECL void Read(T& value);

	M_SOCKET_DECL void Read(std::string& value);

	template<typename T>
	Buffer& operator >> (T& value){
		Read(value);
		return *this;
	}

	M_SOCKET_DECL void Swap(Buffer& buffer);

protected:
	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);

private:
	_data_ _data;
};

M_SOCKET_DECL Buffer::Buffer()
{
	_data._pos = _data._offset = (0);
	_data._size = M_BUFFER_DEFAILT_SIZE;
	_data._data = (s_byte_t*)g_malloc(_data._size);
}

M_SOCKET_DECL Buffer::Buffer(s_uint32_t hdrlen) {
	// hdrlen ���ܴ���M_BUFFER_DEFAILT_SIZE
	_data._pos = _data._offset = (0);
	_data._size = M_BUFFER_DEFAILT_SIZE;
	_data._data = (s_byte_t*)g_malloc(_data._size);
	if (hdrlen <= _data._size)
		_data._pos += hdrlen;
}

M_SOCKET_DECL Buffer::~Buffer(){
	g_free(_data._data);
}

M_SOCKET_DECL void Buffer::Clear() {
	_data._pos = _data._offset = (0);
}

M_SOCKET_DECL void Buffer::RemoveData(s_uint32_t len) {
	if (_data._offset + len > _data._pos)
		_data._offset = _data._pos;
	_data._offset += len;
}

M_SOCKET_DECL s_byte_t* Buffer::Data(){
	return _data._data + _data._offset;
}

M_SOCKET_DECL const s_byte_t* Buffer::Data()const{
	return _data._data + +_data._offset;
}

M_SOCKET_DECL s_byte_t* Buffer::Raw() {
	return _data._data;
}

M_SOCKET_DECL const s_byte_t* Buffer::Raw()const {
	return _data._data;
}

M_SOCKET_DECL s_uint32_t Buffer::Capacity()const{
	return _data._size;
}

M_SOCKET_DECL s_uint32_t Buffer::Size()const{
	return _data._pos;
}

M_SOCKET_DECL s_uint32_t Buffer::Length()const{
	return (_data._pos - _data._offset);
}

M_SOCKET_DECL void Buffer::Write(const void* data, s_uint32_t len)
{
	if (_data._pos + len > _data._size)
	{
		_data._size = M_BUFFER_DEFAILT_SIZE*((_data._pos + len) / M_BUFFER_DEFAILT_SIZE + 1);
		s_byte_t* ptmp = (s_byte_t*)g_malloc(_data._size);
		g_memcpy(ptmp, _data._data, _data._pos);
		g_free(_data._data);
		_data._data = ptmp;
	}
	g_memcpy(_data._data + _data._pos, data, len);
	_data._pos += len;
}

template<typename T>
M_SOCKET_DECL void Buffer::Write(T const& value){
	Write((void*)&value, sizeof(T));
}

M_SOCKET_DECL void Buffer::Write(std::string const& value) {
	s_uint32_t len = value.length();
	Write((void*)&len, sizeof(s_uint32_t));
	Write((void*)value.c_str(), len);
}

M_SOCKET_DECL void Buffer::Read(void* data, s_uint32_t len){
	if (_data._offset + len > _data._pos)
		return;

	g_memcpy(data, _data._data + _data._offset, len);
	_data._offset += len;
}

template<typename T>
M_SOCKET_DECL void Buffer::Read(T& value){
	Read((void*)&value, sizeof(T));
}

M_SOCKET_DECL void Buffer::Read(std::string& value) {
	s_uint32_t len = 0;
	Read(len);
	if (len > 0) {
		char* tmp = (char*)malloc(len + 1);
		Read(tmp, len);
		tmp[len] = 0;
		value = tmp;
		free(tmp);
	}
}

M_SOCKET_DECL void Buffer::Swap(Buffer& buffer) {
	_data_ d = this->_data;
	this->_data = buffer._data;
	buffer._data = d;
}

M_SOCKET_NAMESPACE_END
#endif