cmake_minimum_required(VERSION 3.16)

project(NetTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(INCLUDE_DIR
  ${CMAKE_SOURCE_DIR}/src/
  ${CMAKE_SOURCE_DIR}/external/gflags/include/
  ${CMAKE_SOURCE_DIR}/external/glog/include/
  ${CMAKE_SOURCE_DIR}/external/npcap/include/
  ${CMAKE_SOURCE_DIR}/external/fmt/include/
  ${CMAKE_SOURCE_DIR}/external/json/include/
  ${CMAKE_SOURCE_DIR}/external/magic_enum/include/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/msvc/
  ${CMAKE_SOURCE_DIR}/external/boost/include/boost-1_74/
  ${CMAKE_SOURCE_DIR}/external/webview2/include/
  ${CMAKE_SOURCE_DIR}/external/wil/include/
  ${CMAKE_SOURCE_DIR}/external/v8/include/
)
message(STATUS "Include dir -> " ${INCLUDE_DIR})
include_directories(${INCLUDE_DIR})

set(LINK_DIR
  ${CMAKE_SOURCE_DIR}/external/gflags/lib/
  ${CMAKE_SOURCE_DIR}/external/glog/lib/
  ${CMAKE_SOURCE_DIR}/external/npcap/lib/
  ${CMAKE_SOURCE_DIR}/external/fmt/lib/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc_x64_lib/
  ${CMAKE_SOURCE_DIR}/external/boost/lib/
  ${CMAKE_SOURCE_DIR}/external/webview2/lib/
  ${CMAKE_SOURCE_DIR}/external/v8/lib/
)
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

add_library(common INTERFACE)
target_precompile_headers(common INTERFACE ${CMAKE_SOURCE_DIR}/src/prec.h)
target_link_libraries(common INTERFACE fmt.lib gflags_static.lib glog.lib shlwapi.lib ws2_32.lib)

add_library(protocol STATIC
  ${CMAKE_SOURCE_DIR}/src/protocol/type.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/protocol.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/ethernet.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/arp.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/ipv4.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/icmp.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/udp.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/tcp.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/dns.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/http.cpp
)
target_link_libraries(protocol common)

add_library(transport STATIC
  ${CMAKE_SOURCE_DIR}/src/transport/adaptor.cpp
  ${CMAKE_SOURCE_DIR}/src/transport/port-table.cpp
  ${CMAKE_SOURCE_DIR}/src/transport/packet.cpp
  ${CMAKE_SOURCE_DIR}/src/transport/transport.cpp
)
target_link_libraries(transport protocol wpcap.lib iphlpapi.lib)

add_library(widgets STATIC
  ${CMAKE_SOURCE_DIR}/src/submodule/filter.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/packet-listctrl.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/packet-propgrid.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/sniff-ui.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/sniff-frame.cpp
)
target_link_libraries(widgets transport)

add_executable(lsdev ${CMAKE_SOURCE_DIR}/src/tools/lsdev.cpp)
target_link_libraries(lsdev transport)

add_executable(arp ${CMAKE_SOURCE_DIR}/src/tools/arp.cpp)
target_link_libraries(arp transport)

add_executable(ping ${CMAKE_SOURCE_DIR}/src/tools/ping.cpp)
target_link_libraries(ping transport)

add_executable(mtu ${CMAKE_SOURCE_DIR}/src/tools/mtu.cpp)
target_link_libraries(mtu transport)

add_executable(traceroute ${CMAKE_SOURCE_DIR}/src/tools/traceroute.cpp)
target_link_libraries(traceroute transport)

add_executable(dns ${CMAKE_SOURCE_DIR}/src/tools/dns.cpp)
target_link_libraries(dns transport)

add_executable(sniff WIN32 ${CMAKE_SOURCE_DIR}/src/tools/sniff.cpp)
target_link_libraries(sniff widgets)
add_custom_command(TARGET sniff POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/npcap/bin/npcap-1.10.exe
  $<TARGET_FILE_DIR:sniff>
)

add_executable(browser_test
  ${CMAKE_SOURCE_DIR}/src/submodule/browser.cpp
  ${CMAKE_SOURCE_DIR}/src/submodule/browser_test.cpp
)
target_link_libraries(browser_test common WebView2Loader.dll.lib)
add_custom_command(TARGET browser_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/webview2/bin/MicrosoftEdgeWebview2Setup.exe
  ${CMAKE_SOURCE_DIR}/external/webview2/lib/WebView2Loader.dll
  $<TARGET_FILE_DIR:browser_test>
)

add_executable(v8_test ${CMAKE_SOURCE_DIR}/src/submodule/v8_test.cpp)
target_link_libraries(v8_test v8_monolith.lib dbghelp.lib winmm.lib)
