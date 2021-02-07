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

std::string ws2s(const std::wstring &ws, UINT page = CP_ACP);
std::wstring s2ws(const std::string &s, UINT page = CP_ACP);
std::string read_file(const std::wstring &path);
std::pair<int, std::string> exec(const std::wstring &cmd);
