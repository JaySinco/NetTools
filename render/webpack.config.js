const path = require("path")
const PrettierPlugin = require("prettier-webpack-plugin");

module.exports = {
    mode: "production", // development or production
    devtool: "inline-source-map",
    entry: "./src/index.tsx",
    output: {
        path: path.resolve(__dirname, "../bin/resources/"),
        filename: "app.bundle.js"
    },
    resolve: {
        extensions: [".ts", ".tsx", ".js"]
    },
    performance: {
        hints: false
    },
    externals: {
        "react": "React",
        "react-dom": "ReactDOM",
        "antd": "antd",
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: [
                    {
                        loader: "ts-loader",
                        options: {
                            configFile: path.resolve(__dirname, "./tsconfig.json"),
                        }
                    }
                ]
            },
            {
                test: /\.css$/,
                use: ["style-loader", "css-loader"],
              }
        ]
    },
    plugins: [
        new PrettierPlugin()
    ],
}