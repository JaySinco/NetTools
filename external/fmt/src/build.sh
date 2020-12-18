#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
cmake -G "Visual Studio 15 2017 Win64" -T "host=x64"  ../fmt-7.1.3 \
    -DMSVC_RUNTIME="static" \
    -DFMT_TEST=off \
    -DFMT_DOC=off \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
