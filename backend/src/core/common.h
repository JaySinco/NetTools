#pragma once
#include <string>

// convert between utf8-string and wstring
std::string ws2s(const std::wstring &wstr);
std::wstring s2ws(const std::string &str);

// get absolute resources directory path
std::wstring get_resdir();
