#!/bin/bash

INSTALLER=npcap-1.10.exe
SDK=npcap-sdk-1.06.zip

cd ..
curl -LJO -x socks5h://localhost:8172 https://nmap.org/npcap/dist/${SDK} \
&& \
unzip ${SDK} \
&& \
mv Include include && mv Lib _lib && mv _lib/x64 lib \
&& \
rm -rf docs Examples-pcap _lib Examples-remote Npcap_Guide.html ${SDK} \
&& \
mkdir -p bin && cd bin \
&& \
curl -LJO -x socks5h://localhost:8172 https://nmap.org/npcap/dist/${INSTALLER} \
