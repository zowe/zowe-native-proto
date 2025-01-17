import * as fs from "fs";
import { Writable } from "stream";
import { Client, ClientChannel, ConnectConfig } from "ssh2";
import { DeferredPromise } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { IRpcRequest, IRpcResponse } from "./doc";

export class ZSshClient implements Disposable {
    private static readonly SERVER_CMD = "cd zowe-native-proto/golang/ioserver && ./ioserver";

    private mSshClient: Client;
    private mSshStream: ClientChannel | undefined;
    private mResponse = "";
    private mResponseStream: Writable | undefined;
    private sshMutex: DeferredPromise<void> | undefined;

    private constructor() {}

    public static async create(session: SshSession): Promise<ZSshClient> {
        const client = new ZSshClient();
        client.mSshClient = new Client();
        client.mSshClient.connect(this.buildConnectConfig(session));
        client.mSshStream = await new Promise((resolve, reject) => {
            client.mSshClient.on("ready", () => {
                client.mSshClient.shell(false, (err, stream) => {
                    if (err) {
                        reject(err);
                    } else {
                        stream.stderr.on("data", (chunk: Buffer) => {
                            console.log("STDERR:", chunk.toString());
                        });
                        // stream.stdin.on("data", (chunk: Buffer) => {
                        //     console.log("STDIN:", chunk.toString());
                        // });
                        // stream.stdout.on("data", (chunk: Buffer) => {
                        //     console.log("STDOUT:", chunk.toString());
                        // });
                        stream.write(ZSshClient.SERVER_CMD + "\n");
                        // console.log("client ready");
                        resolve(stream);
                    }
                });
            });
        })
        return client;
    }

    public dispose(): void {
        this.mSshClient?.end();
    }

    public [Symbol.dispose](): void {
        this.dispose();
    }

    public async request<T extends IRpcResponse>(request: IRpcRequest, stream?: Writable): Promise<T> {
        await this.sshMutex?.promise;
        this.sshMutex = new DeferredPromise();
        this.mResponse = "";
        this.mResponseStream = stream;

        return new Promise((resolve, reject) => {
            this.mSshStream!.stdin.write(JSON.stringify(request) + "\n");
            this.mSshStream!.stderr.on("data", this.onErrData.bind(this, reject));
            this.mSshStream!.stdout.on("data", this.onOutData.bind(this, (response: any) => resolve(JSON.parse(response))));
        });
    }

    private static buildConnectConfig(session: SshSession): ConnectConfig {
        return {
            host: session.ISshSession.hostname,
            port: session.ISshSession.port,
            username: session.ISshSession.user,
            password: session.ISshSession.password,
            privateKey: session.ISshSession.privateKey ? fs.readFileSync(session.ISshSession.privateKey) : undefined,
            passphrase: session.ISshSession.keyPassphrase,
        };
    }

    private onErrData(reject: typeof Promise["reject"], chunk: Buffer) {
        const error = chunk.toString();
        console.error(error);
        this.requestEnd();
        reject(error);
    }

    private onOutData(resolve: typeof Promise["resolve"], chunk: Buffer) {
        const endsWithNewLine = chunk[chunk.length - 1] === 0x0a;
        if (endsWithNewLine) {
            chunk = chunk.subarray(0, chunk.length - 1);
        }
        if (this.mResponseStream != null) {
            this.mResponseStream.write(chunk);
        } else {
            this.mResponse += chunk;
        }
        if (endsWithNewLine) {
            this.requestEnd();
            resolve(this.mResponse);
        }
    }

    private requestEnd() {
        this.mSshStream!.stderr.removeAllListeners();
        this.mSshStream!.stdout.removeAllListeners();
        this.mResponseStream?.end();
        this.sshMutex?.resolve();
    }
}
