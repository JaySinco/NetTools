#!/bin/bash

cwd=`readlink -f .`
wxbuilder="/c/Program Files (x86)/wxFormBuilder/wxFormBuilder.exe"
fmt="${cwd}/external/.bin/clang-format.exe"
cloc="${cwd}/external/.bin/cloc.exe"

sniff_ui="${cwd}/resources/sniff-ui.fbp"
if [ $(stat -c %Y ${cwd}/src/sniff/ui.cpp) -le $(stat -c %Y ${sniff_ui}) ]; then
    "${wxbuilder}" -g "${sniff_ui}" 2>/dev/null && echo "code generated for ${sniff_ui}"
fi

case $1 in
"")
    target=""
    ${cloc} --quiet "${cwd}/src"
    ;;
clean)
    rm -rf dest/ bin/
    exit 0
    ;;
*)
    target="-t:$1"
    ;;
esac

find "${cwd}/src" -type f -exec ${fmt} -i {} \; \
&& \
mkdir -p dest/ bin/ \
&& \
cd dest/ \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../ \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${cwd}/bin/ \
&& \
${MSVC_BUILD} ${target}
