cmake_minimum_required(VERSION 3.2)

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
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project(NetTools)

configure_msvc_runtime()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(INCLUDE_DIR
  ${CMAKE_SOURCE_DIR}/src/
  ${CMAKE_SOURCE_DIR}/external/libcef/
  ${CMAKE_SOURCE_DIR}/external/gflags/include/
  ${CMAKE_SOURCE_DIR}/external/glog/include/
  ${CMAKE_SOURCE_DIR}/external/npcap/include/
  ${CMAKE_SOURCE_DIR}/external/fmt/include/
  ${CMAKE_SOURCE_DIR}/external/json/include/
  )
message(STATUS "Include dir -> " ${INCLUDE_DIR})
include_directories(${INCLUDE_DIR})

set(LINK_DIR
  ${CMAKE_SOURCE_DIR}/external/libcef/lib/
  ${CMAKE_SOURCE_DIR}/external/gflags/lib/
  ${CMAKE_SOURCE_DIR}/external/glog/lib/
  ${CMAKE_SOURCE_DIR}/external/npcap/lib/
  )
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

add_library(fmt STATIC
  ${CMAKE_SOURCE_DIR}/external/fmt/src/format.cc
  ${CMAKE_SOURCE_DIR}/external/fmt/src/os.cc
  )

add_library(net STATIC
  ${CMAKE_SOURCE_DIR}/src/common.cpp
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

add_executable(ntls WIN32
  ${CMAKE_SOURCE_DIR}/src/core/basic_client.cpp
  ${CMAKE_SOURCE_DIR}/src/core/basic_app.cpp
  ${CMAKE_SOURCE_DIR}/src/core/msg_handler.cpp
  ${CMAKE_SOURCE_DIR}/src/core/ntls.cpp
  )
target_link_libraries(ntls
  net
  libcef.lib
  libcef_dll_wrapper.lib
  )
add_custom_command(TARGET ntls
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/external/libcef/bin/
  $<TARGET_FILE_DIR:ntls>
  )