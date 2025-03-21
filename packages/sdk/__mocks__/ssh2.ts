import { EventEmitter } from "node:stream";
import type { ClientCallback, ConnectConfig } from "ssh2";

export class Client extends EventEmitter {
    public connect(_config: ConnectConfig) {}
    public end() {}
    public exec(_command: string, _callback: ClientCallback) {}
}
