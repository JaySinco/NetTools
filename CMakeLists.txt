cmake_minimum_required(VERSION 3.16)

project(NetTools)

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
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/msvc/
  )
message(STATUS "Include dir -> " ${INCLUDE_DIR})
include_directories(${INCLUDE_DIR})

set(LINK_DIR
  ${CMAKE_SOURCE_DIR}/external/gflags/lib/
  ${CMAKE_SOURCE_DIR}/external/glog/lib/
  ${CMAKE_SOURCE_DIR}/external/npcap/lib/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc_x64_lib/
  )
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

add_library(fmt STATIC
  ${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc
  ${CMAKE_SOURCE_DIR}/external/fmt/src/os.cc
  )

add_library(core STATIC
  ${CMAKE_SOURCE_DIR}/src/core/type.cpp
  ${CMAKE_SOURCE_DIR}/src/core/protocol.cpp
  ${CMAKE_SOURCE_DIR}/src/core/ethernet.cpp
  ${CMAKE_SOURCE_DIR}/src/core/arp.cpp
  ${CMAKE_SOURCE_DIR}/src/core/ipv4.cpp
  ${CMAKE_SOURCE_DIR}/src/core/icmp.cpp
  ${CMAKE_SOURCE_DIR}/src/core/udp.cpp
  ${CMAKE_SOURCE_DIR}/src/core/dns.cpp
  ${CMAKE_SOURCE_DIR}/src/core/packet.cpp
  ${CMAKE_SOURCE_DIR}/src/core/transport.cpp
  )
target_compile_definitions(core PUBLIC _UNICODE)
target_precompile_headers(core PUBLIC ${CMAKE_SOURCE_DIR}/src/prec.h)
target_link_libraries(core
  fmt
  wpcap.lib
  ws2_32.lib
  glog.lib
  gflags.lib
  iphlpapi.lib
  )

add_executable(lsdev ${CMAKE_SOURCE_DIR}/src/cli/lsdev.cpp)
target_link_libraries(lsdev core)

add_executable(sniff ${CMAKE_SOURCE_DIR}/src/cli/sniff.cpp)
target_link_libraries(sniff core)

add_executable(arp ${CMAKE_SOURCE_DIR}/src/cli/arp.cpp)
target_link_libraries(arp core)

add_executable(ping ${CMAKE_SOURCE_DIR}/src/cli/ping.cpp)
target_link_libraries(ping core)

add_executable(mtu ${CMAKE_SOURCE_DIR}/src/cli/mtu.cpp)
target_link_libraries(mtu core)

add_executable(traceroute ${CMAKE_SOURCE_DIR}/src/cli/traceroute.cpp)
target_link_libraries(traceroute core)

add_executable(dns ${CMAKE_SOURCE_DIR}/src/cli/dns.cpp)
target_link_libraries(dns core)

add_executable(gui WIN32
  ${CMAKE_SOURCE_DIR}/src/widgets/type.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/packet-list.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/main-frame-ui.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/main-frame.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/app.cpp
  )
target_link_libraries(gui core)

add_custom_command(TARGET gui
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/resources/autorun.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_slave.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_server.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_client.bat
    ${CMAKE_SOURCE_DIR}/resources/frps.ini
    ${CMAKE_SOURCE_DIR}/resources/frpc.ini
    ${CMAKE_SOURCE_DIR}/external/npcap/bin/npcap.exe
    ${CMAKE_SOURCE_DIR}/external/nssm/bin/nssm.exe
    ${CMAKE_SOURCE_DIR}/external/frp/bin/frps.exe
    ${CMAKE_SOURCE_DIR}/external/frp/bin/frpc.exe
    ${CMAKE_SOURCE_DIR}/external/brook/bin/brook.exe
  $<TARGET_FILE_DIR:gui>
  )
