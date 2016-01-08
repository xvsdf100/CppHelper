#include "stdafx.h"
#include "StringUtility.h"

std::wstring StringUtility::AToWString( std::string str )
{
	std::wstring strOut;
	int num = MultiByteToWideChar(CP_ACP,0,str.data(),-1,NULL,0);  
	num -=1;	//std 不需要终止符号
	strOut.resize(num);

	MultiByteToWideChar(CP_ACP,0,str.data(),-1,(LPWSTR)strOut.data(),num);  
	return strOut;
}

std::string StringUtility::WToAString( std::wstring str )
{
	std::string strOut;
	int num = WideCharToMultiByte(CP_ACP,0,str.data(),-1,NULL,0,NULL,0);
	num -=1;	//std 不需要终止符号
	strOut.resize(num);

	WideCharToMultiByte(CP_ACP,0,str.data(),-1,(LPSTR)strOut.data(),num,NULL,0);  
	return strOut;
}

std::wstring StringUtility::Utf8ToWString( std::string str )
{
	std::wstring strOut;
	int num = MultiByteToWideChar(CP_UTF8,0,str.data(),-1,NULL,0); 
	num -=1;	//std 不需要终止符号
	strOut.resize(num);
	MultiByteToWideChar(CP_UTF8,0,str.data(),-1,(LPWSTR)strOut.data(),num);
	return strOut;
}

std::string StringUtility::Utf8ToString( std::string str )
{
	std::wstring strWide = Utf8ToWString(str);
	return WToAString(strWide);
}

std::string StringUtility::WStringToUtf8( std::wstring str )
{
	std::string strOut;
	int num = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, 0);
	num -=1;	//std 不需要终止符号
	strOut.resize(num);
	::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, (LPSTR)strOut.data(), num, NULL, 0);
	return strOut;
}

int StringUtility::WSTringToInt( std::wstring str )
{
	int n = 0;
	n = _wtoi(str.data());
	return n;
}

int StringUtility::StringToInt( std::string str )
{
	int n = 0;
	n = atoi(str.data());
	return n;
}

std::string StringUtility::IToString( int n )
{
	char buf[256] = {0};
	itoa(n,buf,10);
	return std::string(buf);
}

std::wstring StringUtility::IToWString( int n )
{
	wchar_t buf[256] = {0};
	_itow(n,buf,10);
	return std::wstring(buf);
}

