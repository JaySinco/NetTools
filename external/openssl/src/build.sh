#!/bin/bash

cwd=`readlink -f .`
source "../../../source_in_bash_profile.sh"

cd openssl-OpenSSL_1_1_1j

perl Configure VC-WIN64A no-shared --openssldir="${cwd}/../" --prefix="${cwd}/../"

# 以下需要在VS本机工具命令行中执行
# nmake
# nmake install
