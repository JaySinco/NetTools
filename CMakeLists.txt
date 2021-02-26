cmake_minimum_required(VERSION 3.16)

project(NetTools)

set(CMAKE_CXX_STANDARD 20)
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
  ${CMAKE_SOURCE_DIR}/external/libuv/include/
  ${CMAKE_SOURCE_DIR}/external/zlib/include/
  ${CMAKE_SOURCE_DIR}/external/openssl/include/
  ${CMAKE_SOURCE_DIR}/external/uwebsockets/include/
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
  ${CMAKE_SOURCE_DIR}/external/libuv/lib/
  ${CMAKE_SOURCE_DIR}/external/zlib/lib/
  ${CMAKE_SOURCE_DIR}/external/openssl/lib/
)
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

file(GLOB_RECURSE uwebsockets_SRC CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/external/uwebsockets/lib/*.c
  ${CMAKE_SOURCE_DIR}/external/uwebsockets/lib/*.cpp
)
add_library(uwebsockets STATIC ${uwebsockets_SRC})
target_compile_definitions(uwebsockets PUBLIC LIBUS_USE_OPENSSL)
target_compile_options(uwebsockets PUBLIC /wd4138)
target_include_directories(uwebsockets PRIVATE ${CMAKE_SOURCE_DIR}/external/uwebsockets/lib/)
target_link_libraries(uwebsockets
  libcrypto.lib libssl.lib uv_a.lib zlibstatic.lib crypt32.lib psapi.lib userenv.lib iphlpapi.lib
)

add_library(common STATIC
  ${CMAKE_SOURCE_DIR}/src/common.cpp
)
target_precompile_headers(common PUBLIC ${CMAKE_SOURCE_DIR}/src/prec.h)
target_link_libraries(common fmt.lib gflags_static.lib glog.lib shlwapi.lib ws2_32.lib)

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
add_custom_command(TARGET sniff POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/npcap/bin/npcap-1.10.exe
  $<TARGET_FILE_DIR:sniff>
)

add_executable(module_test
  ${CMAKE_SOURCE_DIR}/src/submodule/browser.cpp
  ${CMAKE_SOURCE_DIR}/src/submodule/module_test.cpp
)
target_link_libraries(module_test common uwebsockets WebView2Loader.dll.lib)
add_custom_command(TARGET module_test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/webview2/bin/MicrosoftEdgeWebview2Setup.exe
  ${CMAKE_SOURCE_DIR}/external/webview2/lib/WebView2Loader.dll
  $<TARGET_FILE_DIR:module_test>
)

add_executable(v8_test ${CMAKE_SOURCE_DIR}/src/submodule/v8_test.cpp)
target_link_libraries(v8_test common v8_monolith.lib dbghelp.lib winmm.lib)
add_custom_command(TARGET v8_test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/v8/bin/d8.exe
  $<TARGET_FILE_DIR:v8_test>
)
