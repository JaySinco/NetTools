#pragma once
#include <winsock2.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <chrono>
#include <fmt/ranges.h>
#include <fmt/format.h>
#define SPDLOG_FMT_EXTERNAL
#define SPDLOG_COMPILED_LIB
#include "spdlog/spdlog.h"
#include <json.hpp>
#include <gflags/gflags.h>
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#undef max
#undef min

using namespace fmt::literals;
using namespace std::chrono_literals;
using json = nlohmann::ordered_json;
