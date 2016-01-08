#ifndef STRING_UTILITY_H
#define STRING_UTILITY_H

/************************************************************************/
/* 说明：增加std字符串处理功能                                          */
/* 作者：xjm															*/
/* 时间：2015年12月19日 21:16:55										*/
/************************************************************************/
#include <string>

class StringUtility
{
public:
	static std::wstring		AToWString(std::string str);
	static std::string		WToAString(std::wstring str);
	static std::wstring		Utf8ToWString(std::string str);
	static std::string		Utf8ToString(std::string str);
	static std::string		WStringToUtf8(std::wstring str);

	static int				WSTringToInt(std::wstring str);
	static int				StringToInt(std::string str);
	static std::string		IToString(int n);
	static std::wstring		IToWString(int n);
};
#endif