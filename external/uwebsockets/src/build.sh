#!/bin/bash

source "../../../source_in_bash_profile.sh"

uWebSockets=uWebSockets-18.23.0

mkdir -p ../include/uwebsockets/
cp -r ${uWebSockets}/src/* ../include/uwebsockets/
cp -r ${uWebSockets}/uSockets/src/libusockets.h ../include/
mkdir -p ../lib
cp -r ${uWebSockets}/uSockets/src/* ../lib/
