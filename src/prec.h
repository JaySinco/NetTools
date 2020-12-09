#pragma once
#include <winsock2.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <fmt/ranges.h>
#include <fmt/format.h>
#include <json.hpp>
#include <gflags/gflags.h>
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#undef max
#undef min

using namespace fmt::literals;
using json = nlohmann::ordered_json;
