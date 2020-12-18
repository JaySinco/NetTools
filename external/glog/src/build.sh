#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../glog-0.4.0 \
    -DWITH_GFLAGS=on \
    -Dgflags_DIR="../../gflags" \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
