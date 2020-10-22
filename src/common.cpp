#include <windows.h>
#include <sstream>
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

std::string string_join(const std::vector<std::string> &svec, const std::string &delimit)
{
    if (svec.empty()) return "";
    std::ostringstream ss;
    ss << svec[0];
    for (int i = 1; i < svec.size(); ++i) {
        ss << delimit << svec[i];
    }
    return ss.str();
}

std::vector<std::string> string_split(const std::string &str, const std::string &delimit,
                                      bool ignore_empty)
{
    if (str.empty()) return {};
    if (delimit.empty()) return {1, str};

    std::vector<std::string> svec;
    size_t index = std::string::npos;
    size_t last_pos = 0;
    while ((index = str.find(delimit, last_pos)) != std::string::npos) {
        if (index != last_pos || !ignore_empty)
            svec.push_back(str.substr(last_pos, index - last_pos));
        last_pos = index + delimit.size();
    }
    std::string last_one = str.substr(last_pos);
    if (!last_one.empty() || !ignore_empty) svec.push_back(last_one);

    return svec;
}