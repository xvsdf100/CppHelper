#include "stdafx.h"
#include "INIHelper.h"

CINIFile::CINIFile( std::wstring strPath )
{
    m_strFilePath = strPath;
}

CINIFile::~CINIFile()
{

}

std::wstring CINIFile::GetString( std::wstring strSection,std::wstring strKey )
{
    std::wstring strValue;
    strValue.resize(MAX_PATH);
    GetPrivateProfileString(strSection.data(),strKey.data(),_T(""),(wchar_t*)strValue.data(),MAX_PATH,m_strFilePath.data());
    return strValue;
}

int CINIFile::GetInt( std::wstring strSection,std::wstring strKey )
{
    return GetPrivateProfileInt(strSection.data(),strKey.data(),0,m_strFilePath.data());
}

LONGLONG CINIFile::GetInt64( std::wstring strSection,std::wstring strKey )
{
    std::wstring strValue =GetString(strSection,strKey);
    return _wtoi64(strValue.data());
}

BOOL CINIFile::SetString( std::wstring strSection,std::wstring strKey,std::wstring strValue )
{
    return WritePrivateProfileString(strSection.data(),strKey.data(),strValue.data(),m_strFilePath.data());
}

BOOL CINIFile::SetInt( std::wstring strSection,std::wstring strKey,LONG iValue )
{
    std::wstring strValue;
    strValue.resize(MAX_PATH);
    swprintf_s((wchar_t*)strValue.data(),strValue.capacity(),L"%d",iValue);
    return SetString(strSection,strKey,strValue);
}

BOOL CINIFile::SetInt64( std::wstring strSection,std::wstring strKey,LONGLONG iValue )
{
    std::wstring strValue;
    strValue.resize(MAX_PATH);
    swprintf_s((wchar_t*)strValue.data(),strValue.capacity(),L"%I64d",iValue);
    return SetString(strSection,strKey,strValue);
}