#!/bin/bash

cwd=`readlink -f .`
wxbuilder="/c/Program Files (x86)/wxFormBuilder/wxFormBuilder.exe"
fmt="${cwd}/external/.bin/clang-format.exe"
cloc="${cwd}/external/.bin/cloc.exe"

function generate_ui() {
    fbp="${cwd}/resources/$1-ui.fbp"
    if [ $(stat -c %Y ${cwd}/src/widgets/$1-ui.cpp) -le $(stat -c %Y ${fbp}) ]; then
        "${wxbuilder}" -g "${fbp}" 2>/dev/null && echo "code generated for ${fbp}"
    fi
}

ui_list=("sniff")
for ui in ${ui_list[@]}; do
    generate_ui ${ui}
done

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

BUILD_TYPE=`echo ${MSVC_BUILD_TYPE} | tr '[:lower:]' '[:upper:]'`

find "${cwd}/src" -type f -exec ${fmt} -i {} \; \
&& \
mkdir -p dest/ bin/ \
&& \
cd dest/ \
&& \
${CMAKE_CMD} "${CMAKE_GENERATOR}" ../ \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${BUILD_TYPE}=${cwd}/bin/ \
&& \
${MSVC_BUILD} ${target}
