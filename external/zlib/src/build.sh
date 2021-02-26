#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../zlib-1.2.11 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
