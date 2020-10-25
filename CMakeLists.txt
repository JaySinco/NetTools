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

add_library(net STATIC
  ${CMAKE_SOURCE_DIR}/src/net/common.cpp
  ${CMAKE_SOURCE_DIR}/src/net/spec.cpp
  ${CMAKE_SOURCE_DIR}/src/net/net.cpp
  )
target_link_libraries(net
  fmt
  wpcap.lib
  ws2_32.lib
  glog.lib
  gflags.lib
  iphlpapi.lib
  )

add_executable(ntls WIN32
  ${CMAKE_SOURCE_DIR}/src/widgets/main-frame.cpp
  ${CMAKE_SOURCE_DIR}/src/widgets/ntls.cpp
  )
target_link_libraries(ntls net)
target_compile_definitions(ntls PRIVATE _UNICODE)
target_precompile_headers(ntls
  PRIVATE ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/wx/wxprec.h
  )
add_custom_command(TARGET ntls
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/external/npcap/bin/npcap-0.9995.exe
  $<TARGET_FILE_DIR:ntls>
  )

add_executable(lsdev ${CMAKE_SOURCE_DIR}/src/net/lsdev.cpp)
target_link_libraries(lsdev net)

add_executable(sniff ${CMAKE_SOURCE_DIR}/src/net/sniff.cpp)
target_link_libraries(sniff net)

add_executable(arp ${CMAKE_SOURCE_DIR}/src/net/arp.cpp)
target_link_libraries(arp net)

add_executable(ping ${CMAKE_SOURCE_DIR}/src/net/ping.cpp)
target_link_libraries(ping net)

add_executable(traceroute ${CMAKE_SOURCE_DIR}/src/net/traceroute.cpp)
target_link_libraries(traceroute net)

add_executable(dns ${CMAKE_SOURCE_DIR}/src/net/dns.cpp)
target_link_libraries(dns net)
