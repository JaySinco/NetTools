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

set(INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/3rdparty/gflags/include/
    ${CMAKE_SOURCE_DIR}/3rdparty/glog/include/
    ${CMAKE_SOURCE_DIR}/3rdparty/npcap/include/
    )
message(STATUS "Include dir -> " ${INCLUDE_DIR})
include_directories(${INCLUDE_DIR})

set(LINK_DIR
    ${CMAKE_SOURCE_DIR}/3rdparty/gflags/lib/
    ${CMAKE_SOURCE_DIR}/3rdparty/glog/lib/
    ${CMAKE_SOURCE_DIR}/3rdparty/npcap/lib/
    )
message(STATUS "Link dir -> " ${LINK_DIR})
link_directories(${LINK_DIR})

add_library(net STATIC ${CMAKE_SOURCE_DIR}/src/net.cpp)
target_link_libraries(net
    wpcap.lib
    ws2_32.lib
    glog.lib
    gflags.lib
    )

add_executable(lsdev ${CMAKE_SOURCE_DIR}/src/lsdev.cpp)
target_link_libraries(lsdev net)

add_executable(sniff ${CMAKE_SOURCE_DIR}/src/sniff.cpp)
target_link_libraries(sniff net)