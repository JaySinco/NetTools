#!/bin/bash

cwd=`readlink -f .`

cd boost_1_74_0

if [ ! -f "b2" ]; then
    ./bootstrap.bat
fi

./b2 --with-system --with-thread --with-filesystem --with-chrono --with-date_time --with-regex \
    --with-serialization --with-program_options --with-test --with-timer --with-atomic --with-context \
    --build-dir="${cwd}/build/" \
    --prefix="${cwd}/../" \
    toolset=msvc variant=release \
    link=static runtime-link=static threading=multi \
    architecture=x86 address-model=64 \
    install
