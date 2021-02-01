/*************************************************************************************************************
 *************************************************************************************************************
 * WEB SOCKET
 ************************************************************************************************************
 ************************************************************************************************************/
'use strict';

class WS {

    constructor(debugHost) {
        this._private = {
            uri: undefined,
            ws: undefined,
            debugHost: undefined,
            wsRetries: 0,
            onOpen: [],
            onClose: [],
            onTextReceive: null,
            onBinReceive: null,
            finished: false,
            opening: false,
        };
        this._events = {
            onopen: (evt) => {
                this._private.opening = false;
                for(let f of this._private.onOpen) f();
            },
            onclose: (evt) => {
                for(let f of this._private.onClose) f();
                if(!this._private.finished) {
                    setTimeout(() => {
                        this.open({inst: this});
                    }, 1000);
                }
            },
            onerror: (evt) => {
                if(!this._private.finished) {
                    setTimeout(() => {
                        this._private.opening = false;
                        this.close();
                    }, 1000);
                }
            },
            onmessage: (evt) => {
                if(typeof(evt.data) == 'string' && this._private.onTextReceive) {
                    this._private.onTextReceive(evt.data);
                }
                if(typeof(evt.data) == 'object' && this._private.onBinReceive) {
                    this._private.onBinReceive(evt.data);
                }
            }
        };

        this._private.debugHost = debugHost;

    }

    addOnOpen(onOpen) {
        this._private.onOpen.push(onOpen);
    }

    addOnClose(onClose) {
        this._private.onClose.push(onClose);
    }

    setOnTextReceive(onTextReceive) {
        this._private.onTextReceive = onTextReceive;
    }

    setOnBinReceive(onBinReceive) {
        this._private.onBinReceive = onBinReceive;
    }

    open({uri, inst}) {
        if(this._private.opening) {
            return;
        }
        this._private.opening = true;
        if(inst != this) {
            this._private.finished = false;
        }
        if(uri) {
            this._private.uri = uri;
        }
        this.close();
        if(this._private.finished) {
            return;
        }
        this._private.ws = new WebSocket("ws://" + (this._private.debugHost ? this._private.debugHost : location.host) + (this._private.uri ? this._private.uri : ""));
        this._private.ws.binaryType = 'arraybuffer';
        this._private.ws.onopen = this._events.onopen;
        this._private.ws.onclose = this._events.onclose;
        this._private.ws.onerror = this._events.onerror;
        this._private.ws.onmessage = this._events.onmessage;
    }

    send(data) {
        if(this._private.ws && this._private.ws.readyState == 1) {
            this._private.ws.send(data);
        }
    }

    close() {
        if(this._private.ws) {
            this._private.ws.onopen = null;
            this._private.ws.onclose = null;
            this._private.ws.onerror = null;
            this._private.ws.onmessage = null;
            this._private.ws.close();
            this._private.ws = null;
        }
    }
}
