'use strict'

let ws_ = new WS();
let connected = false;

ws_.addOnOpen(() => {
    connected = true;
    document.getElementsByClassName("websocket-state")[0].innerHTML = "WebSocket state: CONNECTED";
});

ws_.addOnClose(() => {
    connected = false;
    document.getElementsByClassName("websocket-state")[0].innerHTML = "WebSocket state: disconnected";
});

ws_.setOnBinReceive((data) => {
    let dv = new DataView(data);
    let offset = 0;
    let out = "";
    for(let i = 0; i < dv.byteLength; i++) {
        out += dv.getUint8(offset++) + (i == (dv.byteLength - 1) ? "" : ",");
    }
    document.getElementById("websocket-bin-log").value += out + "\r\n";
    console.log(data);
});

ws_.setOnTextReceive((data) => {
    document.getElementById("websocket-text-log").value += data + "\r\n";
    console.log(data);
});

ws_.open({ uri: "/ws" });

function wsBinSend() {
    if(!connected) {
        alert("web socket disconnected!");
        return;
    }
    let val = document.getElementById("ws-bin").value.split(',');
    let outBuf = new ArrayBuffer(val.length);
    let dv = new DataView(outBuf);
    let offset = 0;
    for(let v of val) {
        dv.setUint8(offset++, parseInt(v));
    }
    ws_.send(outBuf);
}

function wsTextSend() {
    ws_.send(document.getElementById("ws-text").value);
}