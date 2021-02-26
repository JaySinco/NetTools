#!/bin/bash

source "../../../source_in_bash_profile.sh"

installer=npcap-1.10.exe
sdk=npcap-sdk-1.06.zip

cd ..
${DOWNLOAD} -O https://nmap.org/npcap/dist/${sdk} \
&& \
unzip ${sdk} \
&& \
mv Include _include && mv _include include \
&& \
mv Lib _lib && mv _lib/x64 lib \
&& \
rm -rf docs Examples-pcap _lib Examples-remote Npcap_Guide.html ${sdk} \
&& \
mkdir -p bin && cd bin \
&& \
${DOWNLOAD} -O https://nmap.org/npcap/dist/${installer} \
