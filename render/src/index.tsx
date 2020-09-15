import React from 'react';
import ReactDOM from "react-dom";
import { App } from "./components/app"
import { message } from 'antd';
import { ipcRender } from "./ipc";
import "./css/style.css";

async function handleGlobalShortcut(ev: KeyboardEvent) {
    switch (ev.code) {
    case "F6":
        const key = "loading_pack_render";
        try {
            message.loading({ content: "packing render...", key, duration: 0 });
            await ipcRender.send("pack_render");
            await message.success({ content: "render packed, reloading browser in 1s", key, duration: 1 });
            location.reload();
        }
        catch (e) {
            message.error({ content: `${e}`, key, duration: 3 });
        }
        break;
    }
}

document.addEventListener('keyup', handleGlobalShortcut);
ReactDOM.render(<App />, document.getElementById('root'));
