#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../fmt-7.1.3 \
    -DFMT_TEST=off \
    -DFMT_DOC=off \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
