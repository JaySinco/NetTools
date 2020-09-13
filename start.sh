#!/bin/bash

use_browser="false"
while getopts "e" opt; do
    case $opt in
    e)
        use_browser="true"
        ;;
    esac
done

cd bin
if [ ${use_browser} = "true" ]; then
    start ./resources/index.html
else
    start ./ntls.exe
fi