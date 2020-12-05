#!/bin/bash

current_dir=`readlink -f .`

cd boost_1_74_0
if [ ! -f "b2" ]; then
    ./bootstrap.bat
fi
./b2 --with-system --with-thread --with-filesystem --with-chrono \
    --with-program_options --with-test --with-timer --with-atomic \
    --build-dir="${current_dir}/build/" \
    --prefix="${current_dir}/../" \
    toolset=msvc variant=release \
    link=static runtime-link=static threading=multi \
    architecture=x86 address-model=64 \
    install
