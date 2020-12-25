#pragma once
#define UNICODE
#define _UNICODE
#include <winsock2.h>
#include <chrono>
#include <codecvt>
#include <json.hpp>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <gflags/gflags.h>
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#undef max
#undef min

using namespace fmt::literals;
using namespace std::chrono_literals;
using json = nlohmann::ordered_json;

#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

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

class util
{
public:
    static std::string ws2s(const std::wstring &wstr)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        return myconv.to_bytes(wstr);
    }

    static std::wstring s2ws(const std::string &str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        return myconv.from_bytes(str);
    }
};
