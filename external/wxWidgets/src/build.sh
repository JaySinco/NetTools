#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../wxWidgets-3.1.4 \
    -DwxBUILD_SHARED=OFF \
    -DwxBUILD_TESTS=OFF \
    -DwxBUILD_SAMPLES=ALL \
    -DwxBUILD_DEMOS=ON \
    -DwxBUILD_USE_STATIC_RUNTIME=ON \
    -DwxBUILD_MONOLITHIC=OFF \
    -DwxUSE_WEBVIEW_EDGE=ON \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
