#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
cmake -G "Visual Studio 15 2017 Win64" -T "host=x64"  ../gflags-2.2.2 \
    -DMSVC_RUNTIME="static" \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
