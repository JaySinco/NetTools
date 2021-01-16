#!/bin/bash
# install vs2019, win10 sdk, depot_tools
# -- refer to https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md#Setting-up-Windows
# -- refer to https://toyobayashi.github.io/2020/07/04/V8/
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set vs2019_install="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"

# setup env, execute in cmd.exe
# -- refer to https://v8.dev/docs/source-code
gclient
mkdir v8
cd v8
fetch v8
cd v8
git checkout 8.6.51
gclient sync

# build
# -- refer to https://v8.dev/docs/build-gn
python ./tools/dev/v8gen.py x64.release -- \
    v8_monolithic=true \
    v8_use_external_startup_data=false \
    use_custom_libcxx=false \
    is_component_build=false \
    treat_warnings_as_errors=false \
    v8_symbol_level=0

cat ./out.gn/x64.release/args.gn
ninja -C ./out.gn/x64.release v8_monolith
