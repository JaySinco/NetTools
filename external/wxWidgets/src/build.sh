#!/bin/bash

cwd=`readlink -f .`

mkdir -p build/ \
&& \
cd build \
&& \
cmake -G "Visual Studio 15 2017 Win64" -T "host=x64"  ../wxWidgets-3.1.4 \
    -DCMAKE_INSTALL_PREFIX="${cwd}/../" \
    -DwxBUILD_SHARED=OFF \
    -DwxBUILD_TESTS=OFF \
    -DwxBUILD_SAMPLES=ALL \
    -DwxBUILD_DEMOS=ON \
    -DwxBUILD_USE_STATIC_RUNTIME=ON \
    -DwxBUILD_MONOLITHIC=OFF \
&& \
${MSVC_BUILD} && ${MSVC_INSTALL}
