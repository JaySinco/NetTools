#!/bin/bash

source "../../../source_in_bash_profile.sh"

cd ..
mkdir -p include
${DOWNLOAD} -o include/magic_enum.hpp https://github.com/Neargye/magic_enum/releases/download/v0.7.1/magic_enum.hpp
