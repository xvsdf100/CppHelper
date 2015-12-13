/************************************************************************/
/* 说明：HTTP Client 常见请求的封装                                     */
/* 时间：2015年12月9日 22:37:07											*/
/* 作者：xjm															*/
/************************************************************************/

/*
默认短连接，长连接要使用对应的重载函数
*/
#ifndef	HTTPCLIENTHELPER_H
#define	HTTPCLIENTHELPER_H


#include "Wininet.h"
#include <string>
#include <map>
#include <vector>


class HttpClientHelper
{
public:
	typedef std::map<std::string,std::string> HeadItems;
	typedef HeadItems::const_iterator HeadItemsIt;
	typedef std::vector<char>	ByteArray;
	
class HttpClientRepond;

public:
	HttpClientHelper(std::string host,int port = 80);
	~HttpClientHelper();

	bool Connect();
	void Close();

	bool Post(std::string requestObj, const HeadItems* headItems = NULL, const char* data = NULL, int len = 0);
	bool Get(std::string requestObj = "", const HeadItems* headItems = NULL, const char* data = NULL, int len = 0);
	bool Head(std::string requestOb = "");

	
	bool Get(std::string requestObj, HttpClientRepond* repond , const HeadItems* headItems = NULL, const char* data = NULL, int len = 0);

	//获取跳转url,因为wininet是默认自动跳转，有时候我们需要跳转的URL地址，分析一些东西出来
	std::string GetLocationUrl();

	std::string GetRequestObj();

private:
	bool Action(std::string type,std::string requestObj, HttpClientRepond* repond , const HeadItems* headItems = NULL, const char* data = NULL, int len = 0);

public:
	class HttpClientRepond
	{
		
	public:
		HttpClientRepond() { m_statuCode = 0; }
		~HttpClientRepond() {}

		void SetStatuCode(int code);
		void SetHeads(const HeadItems& heads);
		void SetBody(const ByteArray& body);

		void GetBody(ByteArray& body);
		int  GetStatuCode() {return m_statuCode;}

		std::string GetHeadItem(std::string name);
		std::string operator[](std::string name);

	private:
		int			m_statuCode;
		HeadItems	m_heads;
		ByteArray	m_bodys;
	};

private:
	void InitInternet();

	std::string	GetHeardItems(const HeadItems* headItems);

	//读取Head
	void ReadHead(HINTERNET hRequest, HeadItems& heads, int& statuCode);

	//读取BODY数据
	void ReadData(HINTERNET hRequest,ByteArray& bodyData);

	//释放函数
	void CloseRequestInfo(HINTERNET hRequest);

	//获取请求行
	std::string GetRequestLine(const std::string& head);

private:
	void ParseHead(const std::string& head, HeadItems& heads);

	bool ParseLineText(const std::string& line, std::string& name, std::string& value);

	//解析请求行

	void ParsRequestLine(std::string line,std::string& obj);

private:
	HINTERNET		m_hInternetRoot;
	HINTERNET		m_hInternetConnect;
	HINTERNET		m_hSession;				//这个结构很重要
	std::string		m_host;
	int				m_port;
	std::string		m_agent;
};





#endif


