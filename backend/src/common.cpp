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

std::wstring get_curdir()
{
    wchar_t szFilePath[MAX_PATH + 1] = {0};
    GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
    (wcsrchr(szFilePath, L'\\'))[0] = 0;
    return szFilePath;
}

std::wstring get_resdir() { return get_curdir() + L"\\resources"; }

std::wstring get_srcdir()
{
    std::wstring pwd = get_curdir();
    size_t pos = pwd.find_last_of(L"\\");
    return pwd.substr(0, pos);
}