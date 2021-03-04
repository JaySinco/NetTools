#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../libevent-release-2.1.12-stable \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
    -DEVENT__DISABLE_REGRESS=ON \
    -DEVENT__LIBRARY_TYPE=static \
    -DOPENSSL_ROOT_DIR="${cwd}/../../openssl/" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
