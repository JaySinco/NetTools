#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../wxWidgets-3.1.4 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
    -DwxBUILD_SHARED=OFF \
    -DwxBUILD_TESTS=OFF \
    -DwxBUILD_SAMPLES=ALL \
    -DwxBUILD_DEMOS=ON \
    -DwxBUILD_USE_STATIC_RUNTIME=ON \
    -DwxBUILD_MONOLITHIC=OFF \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
