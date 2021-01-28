#pragma once
#include "prec.h"
#include <chrono>

using namespace fmt::literals;
using namespace std::chrono_literals;
using json = nlohmann::ordered_json;

#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::exception &e) { LOG(ERROR) << e.what(); }

#define INIT_LOG(argc, argv)                           \
    FLAGS_logtostderr = 1;                             \
    FLAGS_minloglevel = 0;                             \
    gflags::ParseCommandLineFlags(&argc, &argv, true); \
    google::InitGoogleLogging(argv[0]);

#define GUI_INIT_LOG(argc, argv)                \
    if (AttachConsole(ATTACH_PARENT_PROCESS)) { \
        freopen("CONOUT$", "w+t", stderr);      \
    }                                           \
    INIT_LOG(argc, argv)

inline std::string ws2s(const std::wstring &ws, UINT page = CP_UTF8)
{
    int len = WideCharToMultiByte(page, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto buf = new char[len]{0};
    WideCharToMultiByte(page, 0, ws.c_str(), -1, buf, len, nullptr, nullptr);
    std::string s = buf;
    delete[] buf;
    return s;
}

inline std::wstring s2ws(const std::string &s, UINT page = CP_UTF8)
{
    int len = MultiByteToWideChar(page, 0, s.c_str(), -1, nullptr, 0);
    auto buf = new wchar_t[len]{0};
    MultiByteToWideChar(page, 0, s.c_str(), -1, buf, len);
    std::wstring ws = buf;
    delete[] buf;
    return ws;
}
