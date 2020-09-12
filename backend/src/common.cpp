#include <windows.h>
#include <codecvt>
#include "common.h"

std::string ws2s(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(wstr);
}

std::wstring s2ws(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

std::wstring get_resdir()
{
    wchar_t szFilePath[MAX_PATH + 1] = {0};
    GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
    (wcsrchr(szFilePath, L'\\'))[1] = 0;
    std::wstring rootPath = szFilePath;
    return rootPath + L"resources";
}

std::wstring get_srcdir()
{
    wchar_t szFilePath[MAX_PATH + 1] = {0};
    GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
    (wcsrchr(szFilePath, L'\\'))[0] = 0;
    (wcsrchr(szFilePath, L'\\'))[1] = 0;
    return szFilePath;
}