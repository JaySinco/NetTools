#!/bin/bash

cd node-v12.20.1

# vcbuild.bat release static x64 vs2017

mkdir -p ../../include \
&& \
cp src/*.h ../../include/ \
&& \
mkdir -p ../../include/v8 \
&& \
cp -r deps/v8/include/* ../../include/v8/ \
&& \
mkdir -p ../../include/uv \
&& \
cp -r deps/uv/include/* ../../include/uv/ \
&& \
mkdir -p ../../lib \
&& \
cp out/Release/lib/*.lib ../../lib/ \
&& \
mkdir -p ../../bin \
&& \
cp out/Release/node.exe ../../bin/
