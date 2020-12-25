#!/bin/bash

sdk=master.zip

cd ..
${DOWNLOAD} -o ${sdk} https://codeload.github.com/microsoft/wil/zip/master \
&& \
unzip ${sdk} \
&& \
mv wil-master/include include \
&& \
rm -rf ${sdk} wil-master
