type OnReceiveCallback = (body?: any) => void;
type OnSendSuccessFunc = (response: string) => void;
type OnSendFailedFunc = (err_code: string, err_msg: string) => void;

interface CefQueryArg {
    request: string,
    onSuccess: OnSendSuccessFunc;
    onFailure: OnSendFailedFunc;
}

declare global {
    interface Window {
        cefQuery: (arg: CefQueryArg) => void;
        onReceiveIpcMessage: (request: string) => void; 
    }
}

class IpcRender {
    static ipcMapping: Map<string, OnReceiveCallback> = new Map;

    static onReceiveIpcMessage(request: string): void {
        const parts = request.split(": ");
        if (parts.length !== 2) {
            console.error(`invalid ipc message: ${request}`);
            return;
        }
        if (!IpcRender.ipcMapping.has(parts[0])) {
            console.error(`unregistered ipc message header: ${parts[0]}`);
            return;
        }
        const callback = IpcRender.ipcMapping.get(parts[0])!;
        if (parts[1].length > 0) {
            callback(JSON.parse(parts[1]));
        }
        else {
            callback();
        }
    }

    public on(header: string, callback: OnReceiveCallback) {
        IpcRender.ipcMapping.set(header, callback);
    }

    public off(header: string) {
        IpcRender.ipcMapping.delete(header);
    }

    public clear() {
        IpcRender.ipcMapping.clear();
    }

    public send(header: string, body?: any): Promise<string> {
        return new Promise((resolve, reject) => {
            window.cefQuery({
                request: `${header}: ${body !== undefined ? JSON.stringify(body) : ""}`,
                onSuccess: (response: string) => {
                    resolve(response);
                },
                onFailure: (err_code: string, err_msg: string) => {
                    reject(new Error(`${err_msg}`));
                }
            });
        });
    }
}

window.onReceiveIpcMessage = IpcRender.onReceiveIpcMessage;

export let ipcRender = new IpcRender();