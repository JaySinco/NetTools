import React from 'react';
import ReactDOM from "react-dom";
import { App } from "./components/app"
import { message } from 'antd';
import { ipcRender } from "./ipc";
import "./css/style.css";

async function addGlobalShortcut(ev: KeyboardEvent) {
    switch (ev.code) {
    case "F6":
        const key = "loading_pack_render";
        try {
            message.loading({ content: "packing render...", key, duration: 0 });
            await ipcRender.send("pack_render");
            await message.success({ content: "render packed, reloading browser in 2s", key, duration: 2 });
            location.reload();
        }
        catch (e) {
            message.error({ content: `${e}`, key, duration: 3 })
        }
        break;
    }
}

document.addEventListener('keydown', addGlobalShortcut);
ReactDOM.render(<App/>, document.getElementById('root'));
