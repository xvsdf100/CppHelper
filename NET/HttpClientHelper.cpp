#include "stdafx.h"
#include "HttpClientHelper.h"


void TrimLeft(std::string& str)
{
	for(int i = 0; i < (int)str.length(); i++)
	{
		if(str[i] == ' ')
		{
			str.erase(i, 1);
		}
	}
}

void TrimRight(std::string& str)
{
	int nLen = str.length();
	for(int i = nLen - 1; i >= 0; i--)
	{
		if(str[i] == ' ')
		{
			str.erase(i, 1);
		}
	}
}

void Trim(std::string& str)
{
	TrimLeft(str);
	TrimRight(str);
}

HttpClientHelper::HttpClientHelper( std::string host, int port ):m_agent("Mozilla/5.0 (MSIE 9.0; qdesk 2.4.1266.203; Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko")
{
	m_host = host;
	m_port = port;

	m_hInternetRoot = NULL;
	m_hInternetConnect = NULL;
	m_hSession = NULL;

	InitInternet();
}

HttpClientHelper::~HttpClientHelper()
{
	Close();
}

bool HttpClientHelper::Post( std::string requestObj, const HttpClientHelper::HeadItems* headItems /*= NULL*/, const char* data /*= NULL*/, int len /*= 0*/ )
{
	return Action("POST", requestObj, NULL , headItems, data, len);
}

bool HttpClientHelper::Post( std::string requestObj, HttpClientRepond* repond , const HeadItems* headItems /*= NULL*/, const char* data /*= NULL*/, int len /*= 0*/ )
{
	return Action("POST", requestObj, repond, headItems, data, len);
}

bool HttpClientHelper::Get( std::string requestObj,const HttpClientHelper::HeadItems* HeadItems /*= NULL*/, const char* data /*= NULL*/, int len /*= 0*/ )
{
	HttpClientRepond* pRepond = NULL;
	return Get(requestObj, pRepond, HeadItems, data, len);
}


bool HttpClientHelper::Get( std::string requestObj,HttpClientHelper::HttpClientRepond* repond , const HttpClientHelper::HeadItems* headItems /*= NULL*/, const char* data /*= NULL*/, int len /*= 0*/ )
{
	return Action("GET", requestObj, repond, headItems, data, len);
}


bool HttpClientHelper::Action( std::string type,std::string requestObj, HttpClientRepond* repond , const HeadItems* headItems /*= NULL*/, const char* data /*= NULL*/, int len /*= 0*/ )
{
	//创建一个Request数据对象而已，因为Request。
	//hRequest一个句柄，对应内部的结构，这个只是生成一个结构而已
	
	bool bRet = false;

	if(Connect())
	{
		//int flag = INTERNET_FLAG_RELOAD ;
		int flag = 0;
		if(m_port == 443)	flag |= INTERNET_FLAG_SECURE;
		m_hSession = HttpOpenRequestA(m_hInternetConnect, type.data() ,  requestObj.data(), NULL, NULL, NULL, flag, NULL);
		if(m_hSession)
		{
			std::string heads = GetHeardItems(headItems);

			//分析一下连接个数
			//正真开始连接
			//这里TCP 连接过去，一直等待TCP 返回整个响应包，然后才返回。
			if( HttpSendRequestA(m_hSession, heads.data(), heads.length(), (LPVOID)data, len) )
			{
				if(NULL != repond)
				{
					HttpClientHelper::HeadItems items;
					int code = 0;
					ReadHead(m_hSession,items, code);
					repond->SetHeads(items);
					repond->SetStatuCode(code);

					ByteArray data;
					ReadData(m_hSession,data);
					repond->SetBody(data);
				}

				int nResultCode = GetResultCode();
				if( nResultCode >= 200 && nResultCode <= 400)
				{
					bRet = true;
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				bRet = false;
			}
		}

		//删除一些请求,不能直接关闭，很多信息可以在这对象里面拿到，如果用户
		//CloseRequestInfo(hRequest);
	}

	return bRet;
}


bool HttpClientHelper::Head( std::string requestObj )
{
	return false;
}

void HttpClientHelper::InitInternet()
{
	m_hInternetRoot = InternetOpenA(m_agent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
}

bool HttpClientHelper::Connect()
{
	//HTTPS 
	//这个实际对于HTTP是没有开始正真的建立TCP
	//这个接口含义就有点模糊，算是他自己建立系统与CLIENT 虚拟链接

	if(NULL == m_hInternetConnect)
	{

		m_hInternetConnect = InternetConnectA(m_hInternetRoot, m_host.data(), m_port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
	}
	
	return (NULL != m_hInternetConnect) ? true : false;
}

void HttpClientHelper::Close()
{
	if(NULL != m_hSession)
	{
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}

	if(NULL != m_hInternetConnect)
	{
		InternetCloseHandle(m_hInternetConnect);
		m_hInternetConnect = NULL;
	}

	if(NULL != m_hInternetRoot)
	{
		InternetCloseHandle(m_hInternetRoot);
		m_hInternetRoot = NULL;
	}
}

std::string HttpClientHelper::GetHeardItems( const HttpClientHelper::HeadItems* headItems )
{
	std::string heads;
	if(NULL != headItems)
	{
		for(HeadItemsIt it = headItems->begin(); it != headItems->end(); it++)
		{
			heads += it->first;
			heads += ": ";
			heads += it->second;
			heads += "\r\n";
		}
	}

	return heads;
}


//释放结构
void HttpClientHelper::CloseRequestInfo( HINTERNET hRequest )
{
	//关闭internet request句柄，实质释放系统内部结构
	InternetCloseHandle(hRequest);
}

void HttpClientHelper::ReadData( HINTERNET hRequest,ByteArray& bodyData )
{
	DWORD dwSize = 0;
	DWORD dwOutSize = 0;
	bodyData.clear();
	const DWORD dwBufferLen = 1024 * 8;
	char* lpBuffer = new char[dwBufferLen];
	do 
	{
		if(!InternetReadFile(hRequest, lpBuffer, dwBufferLen, &dwOutSize))
		{
			break;
		}

		if(0 == dwOutSize)	break;

		//性能会不会有问题
		for(DWORD i = 0; i < dwOutSize; i++)
		{
			bodyData.push_back(lpBuffer[i]);
		}

		memset(lpBuffer,0,dwBufferLen);
	} while (true);

	delete lpBuffer;

}

void HttpClientHelper::ReadHead( HINTERNET hRequest, HttpClientHelper::HeadItems& heads, int& statuCode )
{
	
	DWORD dwLen = 0;
	DWORD dwIndex = 0;
	char  codeBuffer[10]  = {0};
	if(HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwLen, &dwIndex) || dwLen > 0)
	{

		std::string strHead;
		strHead.resize(dwLen);
		HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)strHead.data(), &dwLen, &dwIndex);

		dwLen = sizeof(codeBuffer);
		HttpQueryInfoA(hRequest,HTTP_QUERY_STATUS_CODE, (LPVOID)&codeBuffer, &dwLen, &dwIndex);

		statuCode = atoi(codeBuffer);

		ParseHead(strHead, heads);

	}
}

void HttpClientHelper::ParseHead( const std::string& head,HeadItems& heads )
{
	int nClfPos = 0;
	int nFindPos = 0;
	int nNum = 0;
	nClfPos = head.find("\r\n");
	nFindPos = nClfPos + 1;		//过滤掉状态行
	std::string lineText;
	std::string name, value;

	do 
	{
		nClfPos = head.find("\r\n", nFindPos);
		if(-1 == nClfPos)	break;
		nNum = nClfPos - nFindPos;
		lineText = head.substr(nFindPos, nNum);
		if(ParseLineText(lineText, name, value))
		{
			heads[name] = value;
		}

		nFindPos = nClfPos + 1;

	} while (true);
}

bool HttpClientHelper::ParseLineText( const std::string& line, std::string& name, std::string& value )
{
	int nFindPos = 0;
	int nNum = 0;
	name.clear();
	value.clear();
	if(line.length())
	{
		int nPos = line.find(":", nFindPos);

		if(-1 != nPos)
		{
			nNum = nPos - nFindPos;
			name = line.substr(nFindPos, nNum);
			value = line.substr(nPos + 1);

			Trim(name);
			Trim(value);

			return true;
		}
	}

	return false;
}

std::string HttpClientHelper::GetLocationUrl()
{
	std::string locationUrl;
	std::string obj;
	if(m_hSession)
	{
		char buf[1024] = {0};
		DWORD dwSize = 1024;
		DWORD dwIndex = 0;

		if(HttpQueryInfoA(m_hSession, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, buf, &dwSize, &dwIndex))
		{
			HeadItems heads;
			ParseHead(buf, heads);

			HeadItems::iterator it = heads.find("Host");
			if(it != heads.end())
			{
				std::string requestLine = buf;
				locationUrl = it->second;
				requestLine = GetRequestLine(requestLine);
				ParsRequestLine(requestLine, obj);
				locationUrl += obj;
			}
		}
	}

	return locationUrl;
}


std::string HttpClientHelper::GetRequestObj()
{
	std::string obj;
	if(m_hSession)
	{
		char buf[1024] = {0};
		DWORD dwSize = 1024;
		DWORD dwIndex = 0;

		if(HttpQueryInfoA(m_hSession, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, buf, &dwSize, &dwIndex))
		{
			std::string head = buf;
			std::string requestLine = GetRequestLine(head);
			ParsRequestLine(requestLine, obj);
		}
	}

	return obj;
}
void HttpClientHelper::ParsRequestLine( std::string line,std::string& obj )
{
	if(line.length())
	{
		int nPos = line.find("/");
		if(-1 == nPos)	return;

		int nEndPos = line.find(" ", nPos + 1);
		if(-1 == nEndPos)	return;

		obj = line.substr(nPos, nEndPos - nPos);
	}
}

std::string HttpClientHelper::GetRequestLine( const std::string& head )
{
	std::string str = "";
	int nPos = head.find("\r\n");
	if(-1 != nPos)
	{
		str =  head.substr(0, nPos);
	}

	return str;
}

bool HttpClientHelper::SetCookie( const std::string& cookie )
{
	//一定要设置PATH和DOMAIN不然会设置失败
	//InternetSetCookieA("http://.bing.com",NULL,"CODE=123; PATH=/; DOMAIN=bing.com");
	std::string domain = GetHostDomain();
	std::string url = "http://";
	url += domain;
	std::string fullCookie = cookie;
	std::string urlCookieFlag = "; PATH=/; DOMAIN=";
	urlCookieFlag += domain;
	fullCookie += urlCookieFlag;
	//cookie += urlCookieFlag;
	BOOL bVal = InternetSetCookieA(url.data(),NULL,fullCookie.data());
	return bVal ? true : false;
}

std::string HttpClientHelper::GetCookie( const std::string& cookieName )
{
	std::string strOut;
	DWORD num = 0;
	InternetGetCookieA(m_host.data(),cookieName.data(),NULL,&num);
	if(num)
	{
		strOut.resize(num);
		InternetGetCookieA((char*)m_host.data(),(char*)cookieName.data(),(char*)strOut.data(),&num);
	}

	return strOut;
}

std::string HttpClientHelper::GetHostDomain()
{
	std::string strOut;
	int nPos = m_host.find(".");;
	if(nPos != -1)
	{
		strOut = m_host.substr(nPos);
	}

	return strOut;
}

int HttpClientHelper::GetResultCode()
{
	int resultCode = 0;
	char buf[10] = {0};
	DWORD nIndex = 0;
	DWORD dwSize = 10;
	if(NULL != m_hSession)
	{
		HttpQueryInfoA(m_hSession, HTTP_QUERY_STATUS_CODE, buf, &dwSize, &nIndex);
		resultCode = atoi(buf);
	}
	return resultCode;
}


void HttpClientHelper::HttpClientRepond::SetHeads(const HttpClientHelper::HeadItems& heads )
{
	//进行解析，得出对应的头文件
	m_heads = heads;
}

void HttpClientHelper::HttpClientRepond::SetBody( const ByteArray& body)
{
	m_bodys = body;
}

void HttpClientHelper::HttpClientRepond::GetBody( ByteArray& body )
{
	body = m_bodys;
}

void HttpClientHelper::HttpClientRepond::GetBody(std::string& body)
{
	body.assign(&(*m_bodys.begin()),m_bodys.size());
}

void HttpClientHelper::HttpClientRepond::SetStatuCode( int code )
{
	m_statuCode = code;
}

std::string HttpClientHelper::HttpClientRepond::GetHeadItem( std::string name )
{
	std::string value = "";
	HttpClientHelper::HeadItems::iterator it = m_heads.find(name);
	if(it != m_heads.end())
	{
		value = it->second;
	}

	return value;
}

std::string HttpClientHelper::HttpClientRepond::operator[]( std::string name )
{
	return GetHeadItem(name);
}
