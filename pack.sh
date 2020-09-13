#!/bin/bash

current_dir=`readlink -f .`
source_dir=${current_dir}/render
output_dir=${current_dir}/bin/resources

pushd ${source_dir}

if [ ! -d "node_modules" ]; then
    npm install
    if [ $? -ne 0 ]; then
        echo "[ERROR] failed to install node modules, abort!"
        exit
    fi
fi

rm -rf ${output_dir} \
&& \
mkdir -p ${output_dir} \
&& \
npx webpack \
&& \
cp ${source_dir}/node_modules/react/umd/react.production.min.js ${output_dir}/ \
&& \
cp ${source_dir}/node_modules/react-dom/umd/react-dom.production.min.js ${output_dir}/ \
&& \
cp ${source_dir}/node_modules/antd/dist/antd.min.js ${output_dir}/ \
&& \
cp ${source_dir}/node_modules/antd/dist/antd.min.js.map ${output_dir}/ \
&& \
cp ${source_dir}/node_modules/antd/dist/antd.min.css ${output_dir}/ \
&& \
cp -r ${source_dir}/src/pages/*.html ${output_dir}/

popd