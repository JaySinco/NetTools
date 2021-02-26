#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

webview2_sdk=webview2.zip

${DOWNLOAD} -o ${webview2_sdk} https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.664.37 \
&& \
unzip -d wxWidgets-3.1.4/3rdparty/webview2 ${webview2_sdk} \
&& \
rm -rf ${webview2_sdk} \
&& \
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
