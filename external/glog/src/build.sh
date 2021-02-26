#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

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
