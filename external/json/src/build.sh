#!/bin/bash

source "../../../source_in_bash_profile.sh"

cd ..
mkdir -p include
${DOWNLOAD} -o include/json.hpp https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp 
