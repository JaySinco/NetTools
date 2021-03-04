#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

# have flex & bison in PATH

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../thrift-0.14.0 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
    -DBUILD_TESTING=OFF \
    -DBUILD_JAVA=OFF \
    -DWITH_STATIC_LIB=ON \
    -DZLIB_ROOT="${cwd}/../../zlib/" \
    -DLIBEVENT_ROOT="${cwd}/../../libevent/" \
    -DOPENSSL_ROOT_DIR="${cwd}/../../openssl/" \
    -DBoost_INCLUDE_DIR="${cwd}/../../boost/include/boost-1_74/" \
    -DBOOST_LIBRARYDIR="${cwd}/../../boost/lib/" \
    -DBoost_USE_STATIC_RUNTIME=ON \
    -DBoost_COMPILER="-vc141" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
