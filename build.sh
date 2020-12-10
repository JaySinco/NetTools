#!/bin/bash

# $ cat ~/.ssh/config
# Host github.com
# ProxyCommand connect -S localhost:8172 %h %p

CURRENT_DIR=`readlink -f .`

MSVC_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/HostX86/x86:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/VC/VCPackages:/c/Program Files (x86)/Microsoft SDKs/TypeScript/3.1:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/CommonExtensions/Microsoft/TestWindow:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/CommonExtensions/Microsoft/TeamFoundation/Team Explorer:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/bin/Roslyn:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Team Tools/Performance Tools:/c/Program Files (x86)/Microsoft Visual Studio/Shared/Common/VSPerfCollectionTools/:/c/Program Files (x86)/Microsoft SDKs/Windows/v10.0A/bin/NETFX 4.6.1 Tools/:/c/Program Files (x86)/Windows Kits/10/bin/10.0.17763.0/x86:/c/Program Files (x86)/Windows Kits/10/bin/x86:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community//MSBuild/15.0/bin:/c/Windows/Microsoft.NET/Framework/v4.0.30319:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/Tools/"
PATH="${MSVC_PATH};${PATH}"
FMT="${CURRENT_DIR}/external/clang-format/bin/clang-format.exe"
CLOC="${CURRENT_DIR}/external/cloc/bin/cloc.exe"

if [ ! $1 ]; then
    TARGET_ARG=""
    ${CLOC} --quiet "${CURRENT_DIR}/src"
else
    TARGET_ARG="-t:$1"
fi

find "${CURRENT_DIR}/src" -type f -exec ${FMT} -i {} \; \
&& \
mkdir -p dest/ bin/ \
&& \
pushd dest/ \
&& \
cmake -G "Visual Studio 15 2017 Win64"  ../ \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${CURRENT_DIR}/bin/ \
&& \
MSBuild.exe -p:Configuration=Release -maxcpucount ${TARGET_ARG} *.sln
