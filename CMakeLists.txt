cmake_minimum_required(VERSION 3.16)

macro(configure_msvc_runtime)
  if(MSVC)
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project(NetTools)

add_custom_target(vpn ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/resources/autorun.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_slave.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_server.bat
    ${CMAKE_SOURCE_DIR}/resources/vpn_client.bat
    ${CMAKE_SOURCE_DIR}/resources/frps.ini
    ${CMAKE_SOURCE_DIR}/resources/frpc.ini
    ${CMAKE_SOURCE_DIR}/external/nssm/bin/nssm.exe
    ${CMAKE_SOURCE_DIR}/external/frp/bin/frps.exe
    ${CMAKE_SOURCE_DIR}/external/frp/bin/frpc.exe
    ${CMAKE_SOURCE_DIR}/external/brook/bin/brook.exe
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
  )

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
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/msvc/
  ${CMAKE_SOURCE_DIR}/external/boost/include/boost-1_74/
  )
message(STATUS "Include dir -> " ${INCLUDE_DIR})
include_directories(${INCLUDE_DIR})

set(LINK_DIR
  ${CMAKE_SOURCE_DIR}/external/gflags/lib/
  ${CMAKE_SOURCE_DIR}/external/glog/lib/
  ${CMAKE_SOURCE_DIR}/external/npcap/lib/
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/lib/vc_x64_lib/
  ${CMAKE_SOURCE_DIR}/external/boost/lib/
  )
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

add_library(fmt STATIC
  ${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc
  ${CMAKE_SOURCE_DIR}/external/fmt/src/os.cc
  )

add_library(net STATIC
  ${CMAKE_SOURCE_DIR}/src/net/type.cpp
  ${CMAKE_SOURCE_DIR}/src/net/protocol.cpp
  ${CMAKE_SOURCE_DIR}/src/net/ethernet.cpp
  ${CMAKE_SOURCE_DIR}/src/net/arp.cpp
  ${CMAKE_SOURCE_DIR}/src/net/ipv4.cpp
  ${CMAKE_SOURCE_DIR}/src/net/icmp.cpp
  ${CMAKE_SOURCE_DIR}/src/net/udp.cpp
  ${CMAKE_SOURCE_DIR}/src/net/dns.cpp
  ${CMAKE_SOURCE_DIR}/src/net/packet.cpp
  ${CMAKE_SOURCE_DIR}/src/net/validator.cpp
  ${CMAKE_SOURCE_DIR}/src/net/transport.cpp
  )
target_compile_definitions(net PUBLIC _UNICODE)
target_precompile_headers(net PUBLIC ${CMAKE_SOURCE_DIR}/src/prec.h)
target_link_libraries(net
  fmt
  gflags_static.lib
  glog.lib
  shlwapi.lib
  wpcap.lib
  ws2_32.lib
  iphlpapi.lib
  )

add_executable(lsdev ${CMAKE_SOURCE_DIR}/src/cli/lsdev.cpp)
target_link_libraries(lsdev net)

add_executable(sniff ${CMAKE_SOURCE_DIR}/src/cli/sniff.cpp)
target_link_libraries(sniff net)

add_executable(arp ${CMAKE_SOURCE_DIR}/src/cli/arp.cpp)
target_link_libraries(arp net)

add_executable(ping ${CMAKE_SOURCE_DIR}/src/cli/ping.cpp)
target_link_libraries(ping net)

add_executable(mtu ${CMAKE_SOURCE_DIR}/src/cli/mtu.cpp)
target_link_libraries(mtu net)

add_executable(traceroute ${CMAKE_SOURCE_DIR}/src/cli/traceroute.cpp)
target_link_libraries(traceroute net)

add_executable(dns ${CMAKE_SOURCE_DIR}/src/cli/dns.cpp)
target_link_libraries(dns net)

add_executable(gui WIN32
  ${CMAKE_SOURCE_DIR}/src/graphic/packet-listctrl.cpp
  ${CMAKE_SOURCE_DIR}/src/graphic/packet-propgrid.cpp
  ${CMAKE_SOURCE_DIR}/src/graphic/ui.cpp
  ${CMAKE_SOURCE_DIR}/src/graphic/main-frame.cpp
  ${CMAKE_SOURCE_DIR}/src/graphic/app.cpp
  )
target_link_libraries(gui net)
add_custom_command(TARGET gui
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/external/npcap/bin/npcap.exe
  $<TARGET_FILE_DIR:gui>
  )

add_executable(test ${CMAKE_SOURCE_DIR}/src/cli/test.cpp)
target_link_libraries(test net)

add_custom_target(default ALL
  DEPENDS lsdev sniff arp ping mtu traceroute dns gui test
  )
