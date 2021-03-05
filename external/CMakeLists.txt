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
