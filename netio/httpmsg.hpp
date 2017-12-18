/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/12/2
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_HTTPMSG_INCLUDE
#define M_NETIO_HTTPMSG_INCLUDE

#include <stdio.h>
M_NETIO_NAMESPACE_BEGIN

struct HttpBaseMsg {
	enum {
		E_PARSE_METHOD = 0,
		E_PARSE_URL,
		E_PARSE_VER,
		E_PARSE_STATUS_CODE,
		E_PARSE_STATUS_PHRASE,
		E_PARSE_HEAD,
		E_PARSE_BODY,
		E_PARSE_OVER,
	};

	struct strpos {
		int beg;
		int end;
		static inline void _swap(int& a, int&b) {
			a = a^b;
			b = a^b;
			a = a^b;
		}
		void Swap(strpos& other) {
			_swap(this->beg, other.beg);
			_swap(this->end, other.end);
		}
	};

	SocketLib::Buffer _buffer;
	strpos _header;
	strpos _header2;
	strpos _body;
	int    _flag;
	bool   _assistflag;
	int    _header_iter;
	int    _bodysize;
	std::vector<strpos>
		_header_vec;
	const char* _constr;

	int _Parse1(const char* buffer, int len, bool& hit, char chr) {
		hit = false;
		int pos = 0;
		while (pos < len) {
			if (*(buffer + pos) == chr) {
				hit = true;
				break;
			}
			++pos;
		}
		int copy_len = pos != len ? pos + 1 : len;
		return copy_len;
	}
	int _Parse2(const char* buffer, int len, bool& hit, char chr1, char chr2) {
		hit = false;
		int pos = 0;
		while (pos + 1 < len) {
			if (*(buffer + pos) == chr1
				&& *(buffer + pos + 1) == chr2) {
				hit = true;
				break;
			}
			++pos;
		}
		int copy_len = (pos + 1 != len) ? pos + 2 : len;
		return copy_len;
	}

	HttpBaseMsg() :_constr("Content-Length: ") {
		_header_vec.reserve(30);
	}

	void Clear() {
		_buffer.Clear();
		_header.beg = _header.end
			= _header2.beg = _header2.end
			= _body.beg = _body.end = 0;
		_assistflag = false;
		_header_iter = 0;
		_bodysize = -1;
		_header_vec.clear();
	}

	// copy is slow
	std::string GetHeader()const {
		if (_header.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _header.beg, _header.end - _header.beg);
	}
	// copy is slow
	std::string GetBody()const {
		if (_body.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _body.beg, _body.end - _body.beg);
	}
	std::string NextHeader() {
		if (_header_vec.empty() || _header_iter == _header_vec.size())
			return std::string("");
		else {
			int iter = _header_iter++;
			return std::string(_buffer.Data() + _header_vec[iter].beg, _header_vec[iter].end - _header_vec[iter].beg - 2);
		}
	}
	void InitHeaderIter() {
		_header_iter = 0;
	}
	int GetFlag()const {
		return _flag;
	}
	const char* GetData()const {
		return _buffer.Data();
	}
	int _ParseHead(const char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = 0;
			if (_assistflag) {
				if (*buffer != '\n') {
					_assistflag = false;
					copy_len = _Parse2(buffer, len, hit, '\r', '\n');
				}
				else {
					hit = true;
					copy_len = 1;
				}
			}
			else {
				copy_len = _Parse2(buffer, len, hit, '\r', '\n');
			}
			_buffer.Write(buffer, copy_len);
			if (hit) {
				_assistflag = false;
				int pos = (int)_buffer.Size();
				_header2.end = _header.end = pos;
				_header_vec.push_back(_header2);
				if (_header2.beg + 2 == _header2.end) {
					_flag = E_PARSE_BODY;
					return copy_len + _ParseBody(buffer + copy_len, len - copy_len);
				}
				else {
					_flag = E_PARSE_HEAD;
					_header2.beg = pos;
					_header2.end = 0;
					return copy_len + _ParseHead(buffer + copy_len, len - copy_len);
				}
			}
			if (!_assistflag) {
				int pos = (int)_buffer.Size();
				if (*(_buffer.Data() + pos - 1) == '\r')
					_assistflag = true;
			}
			return copy_len;
		}
		return 0;
	}
	int _ParseBody(const char* buffer, int len) {
		if (len > 0) {
			if (_bodysize == -1) {
				// check "Content-Length:"
				_bodysize = _CheckContentLen();
				if (_bodysize == 0)
					_flag = E_PARSE_OVER;
				else
					_body.beg = _buffer.Size();
			}
			if (_bodysize != 0) {
				int copy_len = _bodysize - (_buffer.Size() - _body.beg);
				if (copy_len > len)
					copy_len = len;
				else
					_flag = E_PARSE_OVER;

				_buffer.Write(buffer, copy_len);
				if (_flag == E_PARSE_OVER)
					_body.end = _buffer.Size();
				return copy_len;
			}
		}
		return 0;
	}
	int _CheckContentLen() {
		const char *pbeg = 0;
		const char *psrc = 0;
		char* pbuf = new char[11];
		char* pbuf2 = pbuf;
		memset(pbuf, 0, 11);
		for (std::vector<strpos>::iterator iter = _header_vec.begin(); iter != _header_vec.end();
			++iter) {
			// include '\r\n'
			if (iter->end - iter->beg < 18)
				continue;
			pbeg = _buffer.Data() + iter->beg;
			psrc = _constr;
			while (*psrc != ' ')
				if (*(pbeg++) != *(psrc++))
					break;
			if (*psrc == ' ') {
				pbeg += 1;
				while (*pbeg != '\r')
					*pbuf++ = *pbeg++;
				int ret = atoi(pbuf2);
				delete[]pbuf2;
				return ret;
			}
		}
		delete[]pbuf2;
		return 0;
	}

};

struct HttpSvrRecvMsg : public HttpBaseMsg{
private:
	strpos _method;
	strpos _url;
	strpos _ver;
	
protected:
	int _ParseMethod(const char* buffer, int len) {
		bool hit = false;
		int copy_len = _Parse1(buffer, len, hit, ' ');
		_buffer.Write(buffer, copy_len);
		if (hit) {
			int pos = (int)_buffer.Size();
			_method.end = pos - 1;
			_flag = E_PARSE_URL;
			_url.beg = pos;
			return copy_len + _ParseUrl(buffer + copy_len, len - copy_len);
		}
		return copy_len;
	}
	int _ParseUrl(const char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = _Parse1(buffer, len, hit, ' ');
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_url.end = pos - 1;
				_flag = E_PARSE_VER;
				_ver.beg = pos;
				return copy_len + _ParseVer(buffer + copy_len, len - copy_len);
			}
			return copy_len;
		}
		return 0;
	}
	int _ParseVer(const char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = 0;
			if (_assistflag) {
				if (*buffer != '\n') {
					_assistflag = false;
					copy_len = _Parse2(buffer, len, hit, '\r', '\n');
				}
				else {
					hit = true;
					copy_len = 1;
				}
			}
			else {
				copy_len = _Parse2(buffer, len, hit, '\r', '\n');
			}
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_ver.end = pos - 2;
				_flag = E_PARSE_HEAD;
				_header2.beg = _header.beg = pos;
				_assistflag = false;
				return copy_len + _ParseHead(buffer + copy_len, len - copy_len);
			}
			if (!_assistflag) {
				int pos = (int)_buffer.Size();
				if (*(_buffer.Data() + pos - 1) == '\r')
					_assistflag = true;
			}
			return copy_len;
		}
		return 0;
	}

public:
	// length is 16
	HttpSvrRecvMsg():HttpBaseMsg() {
		Clear();
	}

	void Clear() {
		HttpBaseMsg::Clear();
		_method.beg = _method.end
			= _url.beg = _url.end
			= _ver.beg = _ver.end = 0;
		_flag = E_PARSE_METHOD;
		
	}

	void Swap(HttpSvrRecvMsg& other) {
		if (this != &other) {
			other._buffer.Swap(this->_buffer);
			other._method.Swap(this->_method);
			other._url.Swap(this->_url);
			other._ver.Swap(this->_ver);
			other._header.Swap(this->_header);
			other._header2.Swap(this->_header2);
			other._body.Swap(this->_body);
			other._header_vec.swap(this->_header_vec);
			strpos::_swap(other._flag, this->_flag);
			strpos::_swap(other._header_iter, this->_header_iter);
			strpos::_swap(other._bodysize, this->_bodysize);
			bool _af = other._assistflag;
			other._assistflag = this->_assistflag;
			this->_assistflag = _af;
		}
	}

	// ����ʹ�õĳ���ֵ
	int Parse(const char* buffer, int len) {
		switch (_flag) {
		case E_PARSE_OVER:
			break;
		case E_PARSE_METHOD:
			return _ParseMethod(buffer, len);
		case E_PARSE_URL:
			return _ParseUrl(buffer, len);
		case E_PARSE_VER:
			return _ParseVer(buffer, len);
		case E_PARSE_HEAD:
			return _ParseHead(buffer, len);
		case E_PARSE_BODY:
			return _ParseBody(buffer, len);
		default:
			break;
		}
		return 0;
	}

	// copy is slow
	std::string GetRequestLine()const {
		if (_ver.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _method.beg, _ver.end - _method.beg);
	}
	// copy is slow
	std::string GetMethod()const {
		if (_method.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _method.beg, _method.end - _method.beg);
	}
	// copy is slow
	std::string GetUrl()const {
		if (_url.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _url.beg, _url.end - _url.beg);
	}
	// copy is slow
	std::string GetVersion()const {
		if (_ver.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _ver.beg, _ver.end - _ver.beg);
	}
	
};

struct HttpSvrSendMsg {
private:
	enum {
		E_VER = 1 << 0,
		E_STATUS_CODE = 1 << 1,
		E_STATUS_PHRASE = 1 << 2,
		E_LOCATION = 1 << 3,
		E_CONT_CODING = 1 << 4,
		E_CONT_TYPE = 1 << 5,
		E_CONT_LANGUAGE = 1 << 6,
		E_DATE = 1 << 7,
		E_SERVER = 1 << 8,
		E_BODY = 1 << 9,
	};

public:
	SocketLib::Buffer* _pbuffer;
	unsigned int _flag;

	~HttpSvrSendMsg() {
		delete _pbuffer;
	}
	HttpSvrSendMsg() {
		_pbuffer = new SocketLib::Buffer;
		_flag = 0;
	}
	SocketLib::Buffer* GetBuffer() {
		return _pbuffer;
	}
	std::string ToString() {
		return std::string(_pbuffer->Data(), _pbuffer->Size());
	}

	bool SetHttpVersion(const char* ver) {
		if (!(_flag&E_VER)) {
			_pbuffer->Write((void*)ver,strlen(ver));
			_pbuffer->Write(' ');
			_flag |= E_VER;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetStatusCode(const char* status) {
		if (!(_flag&E_STATUS_CODE)
			&& _flag&E_VER) {
			_pbuffer->Write((void*)status, strlen(status));
			_pbuffer->Write(' ');
			_flag |= E_STATUS_CODE;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetStatusPhrase(const char* phrase) {
		if (!(_flag&E_STATUS_PHRASE)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE) {
			_pbuffer->Write((void*)phrase, strlen(phrase));
			_pbuffer->Write((void*)"\r\n",2);
			_flag |= E_STATUS_PHRASE;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetLocation(const char* location) {
		if (!(_flag&E_LOCATION)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Location: ",10);
			_pbuffer->Write((void*)location, strlen(location));
			_pbuffer->Write((void*)"\r\n",2);
			_flag |= E_LOCATION;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetContentEncode(const char* encode) {
		if (!(_flag&E_CONT_CODING)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Content-Encoding: ",18);
			_pbuffer->Write((void*)encode, strlen(encode));
			_pbuffer->Write((void*)"\r\n",2);
			_flag |= E_CONT_CODING;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetContentType(const char* type) {
		if (!(_flag&E_CONT_TYPE)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Content-Type: ",14);
			_pbuffer->Write((void*)type, strlen(type));
			_pbuffer->Write((void*)"\r\n",2);
			_flag |= E_CONT_TYPE;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetContentLanguage(const char* language) {
		if (!(_flag&E_CONT_LANGUAGE)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Content-Language: ",18);
			_pbuffer->Write((void*)language, strlen(language));
			_pbuffer->Write((void*)"\r\n",2);
			_flag |= E_CONT_LANGUAGE;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetBody(const char* body, int len) {
		if (!(_flag&E_BODY)) {
			if (!(_flag&E_VER))
				SetHttpVersion("HTTP/1.1");
			if (!(_flag&E_STATUS_CODE))
				SetStatusCode("200");
			if (!(_flag&E_STATUS_PHRASE))
				SetStatusPhrase("OK");
			if (!(_flag&E_CONT_TYPE))
				SetContentType("text/html;charset=utf-8");

			char tmp[20];
			_pbuffer->Write((void*)"Content-Length: ",16);
			snprintf(tmp, 20, "%d", len);
			_pbuffer->Write(tmp, strlen(tmp));
			_pbuffer->Write((void*)"\r\n\r\n", 4);
			_pbuffer->Write((void*)body, len);
			_flag |= E_BODY;
			return true;
		}
		assert(0);
		return false;

		if (!(_flag&E_CONT_LANGUAGE)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {

		}
	}
	bool SetSever(const char* svr) {
		if (!(_flag&E_SERVER)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Server: ");
			_pbuffer->Write((void*)svr, strlen(svr));
			_pbuffer->Write((void*)"\r\n");
			_flag |= E_SERVER;
			return true;
		}
		assert(0);
		return false;
	}
	bool SetDate(const char* date) {
		if (!(_flag&E_DATE)
			&& _flag&E_VER
			&& _flag&E_STATUS_CODE
			&& _flag&E_STATUS_PHRASE) {
			_pbuffer->Write((void*)"Date: ");
			_pbuffer->Write((void*)date, strlen(date));
			_pbuffer->Write((void*)"\r\n");
			_flag |= E_DATE;
			return true;
		}
		assert(0);
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////

struct HttpCliRecvMsg : public HttpBaseMsg {
private:
	strpos _version;
	strpos _statuscode;
	strpos _statusphrase;

protected:
	int _ParseVer(const char* buffer, int len) {
		bool hit = false;
		int copy_len = _Parse1(buffer, len, hit, ' ');
		_buffer.Write(buffer, copy_len);
		if (hit) {
			int pos = (int)_buffer.Size();
			_version.end = pos - 1;
			_flag = E_PARSE_STATUS_CODE;
			_statuscode.beg = pos;
			return copy_len + _ParseStatusCode(buffer + copy_len, len - copy_len);
		}
		return copy_len;
	}
	int _ParseStatusCode(const char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = _Parse1(buffer, len, hit, ' ');
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_statuscode.end = pos - 1;
				_flag = E_PARSE_STATUS_PHRASE;
				_statusphrase.beg = pos;
				return copy_len + _ParseStatusPhrase(buffer + copy_len, len - copy_len);
			}
			return copy_len;
		}
		return 0;
	}
	int _ParseStatusPhrase(const char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = 0;
			if (_assistflag) {
				if (*buffer != '\n') {
					_assistflag = false;
					copy_len = _Parse2(buffer, len, hit, '\r', '\n');
				}
				else {
					hit = true;
					copy_len = 1;
				}
			}
			else {
				copy_len = _Parse2(buffer, len, hit, '\r', '\n');
			}
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_statusphrase.end = pos - 2;
				_flag = E_PARSE_HEAD;
				_header2.beg = _header.beg = pos;
				_assistflag = false;
				return copy_len + _ParseHead(buffer + copy_len, len - copy_len);
			}
			if (!_assistflag) {
				int pos = (int)_buffer.Size();
				if (*(_buffer.Data() + pos - 1) == '\r')
					_assistflag = true;
			}
			return copy_len;
		}
		return 0;
	}

public:
	HttpCliRecvMsg() :HttpBaseMsg() {
		Clear();
	}

	void Clear() {
		HttpBaseMsg::Clear();
		_version.beg = _version.end
			= _statuscode.beg = _statuscode.end
			= _statusphrase.beg = _statusphrase.end = 0;
		_flag = E_PARSE_VER;
	}

	void Swap(HttpCliRecvMsg& other) {
		if (this != &other) {
			other._buffer.Swap(this->_buffer);
			other._version.Swap(this->_version);
			other._statuscode.Swap(this->_statuscode);
			other._statusphrase.Swap(this->_statusphrase);
			other._header.Swap(this->_header);
			other._header2.Swap(this->_header2);
			other._body.Swap(this->_body);
			other._header_vec.swap(this->_header_vec);
			strpos::_swap(other._flag, this->_flag);
			strpos::_swap(other._header_iter, this->_header_iter);
			strpos::_swap(other._bodysize, this->_bodysize);
			bool _af = other._assistflag;
			other._assistflag = this->_assistflag;
			this->_assistflag = _af;
		}
	}

	// ����ʹ�õĳ���ֵ
	int Parse(char* buffer, int len) {
		switch (_flag) {
		case E_PARSE_OVER:
			break;
		case E_PARSE_VER:
			return _ParseVer(buffer, len);
		case E_PARSE_STATUS_CODE:
			return _ParseStatusCode(buffer, len);
		case E_PARSE_STATUS_PHRASE:
			return _ParseStatusPhrase(buffer, len);
		case E_PARSE_HEAD:
			return _ParseHead(buffer, len);
		case E_PARSE_BODY:
			return _ParseBody(buffer, len);
		default:
			break;
		}
		return 0;
	}
	// copy is slow
	std::string GetRespondLine()const {
		if (_statusphrase.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _version.beg, _statusphrase.end - _version.beg);
	}
	// copy is slow
	std::string GetVersion()const {
		if (_version.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _version.beg, _version.end - _version.beg);
	}
	// copy is slow
	std::string GetStatusCode()const {
		if (_statuscode.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _statuscode.beg, _statuscode.end - _statuscode.beg);
	}
	// copy is slow
	std::string GetStatusPhrase()const {
		if (_statusphrase.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _statusphrase.beg, _statusphrase.end - _statusphrase.beg);
	}
};


M_NETIO_NAMESPACE_END
#endif