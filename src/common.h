#pragma once
#include <string>
#include "fmt/core.h"
#define NTLS_FAILED 0
#define NTLS_SUCC 1
#define NTLS_CONTINUE 2
#define NTLS_UNEXPECTED_ERROR -10000
#define NTLS_TIMEOUT_ERROR 10000
#define NTLS_RECV_ERROR_ICMP 10001

template <typename T>
std::string to_string(const T &v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

// convert between utf8-string and wstring
std::string ws2s(const std::wstring &wstr);
std::wstring s2ws(const std::string &str);

// get absolute directory path
std::wstring get_curdir();  // binary current dir
std::wstring get_resdir();  // resources dir
std::wstring get_srcdir();  // code source dir

// split&join string by delimit
std::string string_join(const std::vector<std::string> &svec, const std::string &delimit);
std::vector<std::string> string_split(const std::string &str, const std::string &delimit = "\n",
                                      bool ignore_empty = true);
