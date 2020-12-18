#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../gflags-2.2.2 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
