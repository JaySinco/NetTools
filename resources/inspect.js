const WebSocket = require('ws');
const request = require('request');

function debug(url)
{
    const ws = new WebSocket(url);
    let requestId = 1;

    ws.on('open', () => {
        ws.send(JSON.stringify({ id: requestId++, method: "Debugger.enable" }));
        ws.send(JSON.stringify({ id: requestId++, method: "Runtime.runIfWaitingForDebugger" }));
    });

    ws.on('message', (data) => {
        console.log(`${data}\n`);
        let dataObj = JSON.parse(data)
        if (dataObj.method === "Debugger.scriptParsed" && dataObj.params.url === "evalmachine.<anonymous>") {
            ws.send(JSON.stringify({
                id: requestId++,
                method: "Debugger.setBreakpoint",
                params: { location: { lineNumber: 11, scriptId: dataObj.params.scriptId } },
            }));
        }
    });

    ws.on('error', (data) => {
        console.error(`${data}\n`);
    });
}

request('http://127.0.0.1:9229/json/list', (error, response, body) => {
    if (!error && response.statusCode == 200) {
        let url = JSON.parse(body)[0].webSocketDebuggerUrl;
        console.log(`debug-url: ${url}`);
        debug(url);
    }
    else {
        console.error(`${error}: ${JSON.stringify(body)}`);
    }
});
