#!/bin/bash

cd ..
mkdir -p include
cd include
curl -LJO -x socks5h://localhost:8172 https://github.com/nlohmann/json/releases/download/v3.9.1/json.hpp 
