#!/bin/bash

cwd=`readlink -f .`
wxbuilder="/c/Program Files (x86)/wxFormBuilder/wxFormBuilder.exe"
fmt="${cwd}/external/.bin/clang-format.exe"
cloc="${cwd}/external/.bin/cloc.exe"
uifbp="${cwd}/resources/ui.fbp"

if [ $(stat -c %Y ${cwd}/src/graphic/ui.cpp) -le $(stat -c %Y ${uifbp}) ]; then
    "${wxbuilder}" -g "${uifbp}" 2>/dev/null && echo "code generated for ${uifbp}"
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
