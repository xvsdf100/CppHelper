#include "stdafx.h"
#include "PathHelper.h"

std::wstring PathHelper::GetCurrentMoudlePath()
{
	TCHAR tszModule[MAX_PATH + 1] = { 0 };
	::GetModuleFileName(NULL, tszModule, MAX_PATH);
	std::wstring wMoudleName = tszModule;
	int pos = wMoudleName.rfind(_T('\\'));
	if( pos >= 0 ) wMoudleName = wMoudleName.substr(0, pos + 1);
	return wMoudleName;
}

