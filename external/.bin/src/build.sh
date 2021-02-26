#!/bin/bash

source "../../../source_in_bash_profile.sh"

cd ..
${DOWNLOAD} -o clang-format.exe https://prereleases.llvm.org/win-snapshots/clang-format-6923b0a7.exe \
&& \
${DOWNLOAD} -o cloc.exe https://github.com/AlDanial/cloc/releases/download/1.88/cloc-1.88.exe
