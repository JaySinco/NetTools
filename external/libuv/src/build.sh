#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../libuv-1.41.0 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
    -DBUILD_TESTING=OFF \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL} \
&& \
mv ../../lib/Release/* ../../lib/ \
&& \
rm -r ../../lib/Release/ ../../LICENSE
