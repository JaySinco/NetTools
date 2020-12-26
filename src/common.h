#pragma once
#include "prec.h"
#include <codecvt>
#include <chrono>

using namespace fmt::literals;
using namespace std::chrono_literals;
using json = nlohmann::ordered_json;

#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::exception &e) { LOG(ERROR) << e.what(); }

#define INIT_LOG(argc, argv)                           \
    google::InitGoogleLogging(argv[0]);                \
    gflags::ParseCommandLineFlags(&argc, &argv, true); \
    FLAGS_logtostderr = 1;                             \
    FLAGS_minloglevel = 0;

#define GUI_INIT_LOG(argc, argv)                \
    if (AttachConsole(ATTACH_PARENT_PROCESS)) { \
        freopen("CONOUT$", "w+t", stderr);      \
    }                                           \
    INIT_LOG(argc, argv)

inline std::string ws2s(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(wstr);
}

inline std::wstring s2ws(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}
