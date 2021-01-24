#!/bin/bash

_build_completions()
{
    if [ -d "dest" ]; then
        COMPREPLY+=($(compgen -W "$(find dest -maxdepth 1 -name *.vcxproj | grep -v [[:upper:]] | awk -F [/.] '{print $2}')" -- "${COMP_WORDS[-1]}"))
        COMPREPLY+=($(compgen -W "clean" -- "${COMP_WORDS[-1]}"))
    fi
}

complete -F _build_completions build.sh

export DOWNLOAD="curl -LJ" # -x socks5h://localhost:8172
export CMAKE_CMD="cmake -A x64 -G" # -T clangcl
export CMAKE_GENERATOR="Visual Studio 16 2019"
export MSVC_PATH="\
/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin:\
/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/bin/Hostx64/x64/:\
"
export MSVC_BUILD_TYPE="Debug" # Debug or Release
export MSVC_BUILD="MSBuild.exe -p:Configuration=${MSVC_BUILD_TYPE} -maxcpucount *.sln"
export MSVC_INSTALL="MSBuild.exe -p:Configuration=${MSVC_BUILD_TYPE} INSTALL.vcxproj"
export PATH="${MSVC_PATH};${PATH}"
