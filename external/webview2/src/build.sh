#!/bin/bash

source "../../../source_in_bash_profile.sh"

installer=MicrosoftEdgeWebview2Setup.exe
sdk=webview2.zip

cd ..
mkdir -p bin \
&& \
${DOWNLOAD} -o bin/${installer} https://go.microsoft.com/fwlink/p/?LinkId=2124703 \
&& \
${DOWNLOAD} -o ${sdk} https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.664.37 \
&& \
unzip -d webview2 ${sdk} \
&& \
mv webview2/build/native/include include \
&& \
mv webview2/build/native/x64 lib \
&& \
rm -rf webview2 ${sdk}
