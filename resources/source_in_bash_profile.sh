#!/bin/bash

_build_completions()
{
    if [ -d "dest" ]; then
        COMPREPLY=($(compgen -W "$(find dest -maxdepth 1 -name *.vcxproj | grep -v [[:upper:]] | awk -F [/.] '{print $2}')" -- "${COMP_WORDS[-1]}"))
        COMPREPLY+=("clean")
    fi
}

complete -F _build_completions build.sh

export DOWNLOAD="curl -LJO -x socks5h://localhost:8172"
export CMAKE_CMD="cmake -T host=x64 -G"
export CMAKE_GENERATOR="Visual Studio 15 2017 Win64"
export MSVC_PATH="/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/HostX86/x86:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/VC/VCPackages:/c/Program Files (x86)/Microsoft SDKs/TypeScript/3.1:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/CommonExtensions/Microsoft/TestWindow:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/CommonExtensions/Microsoft/TeamFoundation/Team Explorer:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/bin/Roslyn:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Team Tools/Performance Tools:/c/Program Files (x86)/Microsoft Visual Studio/Shared/Common/VSPerfCollectionTools/:/c/Program Files (x86)/Microsoft SDKs/Windows/v10.0A/bin/NETFX 4.6.1 Tools/:/c/Program Files (x86)/Windows Kits/10/bin/10.0.17763.0/x86:/c/Program Files (x86)/Windows Kits/10/bin/x86:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community//MSBuild/15.0/bin:/c/Windows/Microsoft.NET/Framework/v4.0.30319:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/:/c/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/Tools/"
export MSVC_BUILD="MSBuild.exe -p:Configuration=Release -maxcpucount *.sln"
export MSVC_INSTALL="MSBuild.exe -p:Configuration=Release INSTALL.vcxproj"
export PATH="${MSVC_PATH};${PATH}"
