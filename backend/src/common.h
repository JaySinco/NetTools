#pragma once
#include <string>

#define NTLS_FAILED 0
#define NTLS_SUCC 1
#define NTLS_CONTINUE 2
#define NTLS_UNEXPECTED_ERROR -10000
#define NTLS_TIMEOUT_ERROR 10000
#define NTLS_RECV_ERROR_ICMP 10001

// convert between utf8-string and wstring
std::string ws2s(const std::wstring &wstr);
std::wstring s2ws(const std::string &str);

// get absolute resources directory path
std::wstring get_resdir();
