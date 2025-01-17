import * as fs from "fs";
import { Writable } from "stream";
import { Client, ClientChannel } from "ssh2";
import { DeferredPromise } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { IRpcRequest, IRpcResponse } from "./doc";

export class ZSshClient {
    private static mInstance: ZSshClient;
    private mSshClient: Client;
    private mSshStream: ClientChannel | undefined;
    private mResponse = "";
    private mResponseStream: Writable | undefined;
    private sshMutex: DeferredPromise<void> | undefined;
    private readonly SERVER_CMD = "./zowe-native-proto/golang/ioserver/ioserver";

    private constructor() {}

    public dispose(): void {
        this.mSshClient?.end();
    }

    public static get inst(): ZSshClient {
        this.mInstance ??= new ZSshClient();
        return this.mInstance;
    }

    public async request<T extends IRpcResponse>(session: SshSession, request: IRpcRequest, stream?: Writable): Promise<T> {
        await this.sshMutex?.promise;
        this.sshMutex = new DeferredPromise();
        const _sshClient = await this.client(session);
        this.mResponseStream = stream;

        return new Promise((resolve, reject) => {
            this.mResponse = "";
            this.mSshStream!.stdin.write(JSON.stringify(request) + "\n");
            this.mSshStream!.stderr.on("data", this.onErrData.bind(this, reject));
            this.mSshStream!.stdout.on("data", this.onOutData.bind(this, (response) => resolve(JSON.parse(response))));
        });
    }

    private async client(session: SshSession): Promise<Client> {
        if (this.mSshClient == null) {
            this.mSshClient = new Client();
            this.mSshClient.connect({
                host: session.ISshSession.hostname,
                port: session.ISshSession.port,
                username: session.ISshSession.user,
                password: session.ISshSession.password,
                privateKey: session.ISshSession.privateKey ? fs.readFileSync(session.ISshSession.privateKey) : undefined,
                passphrase: session.ISshSession.keyPassphrase,
            });
            this.mSshStream = await new Promise((resolve, reject) => {
                this.mSshClient.on("ready", () => {
                    this.mSshClient.shell(false, (err, stream) => {
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
                            stream.write(this.SERVER_CMD + "\n");
                            console.log("client ready");
                            resolve(stream);
                        }
                    });
                });
            })
        }
        return this.mSshClient;
    }

    private onErrData(reject: (reason?: any) => void, chunk: Buffer) {
        console.error(chunk.toString());
        this.mSshStream!.end(`exit\n`);
        this.sshMutex?.resolve();
        reject();
    }

    private async onOutData(resolve: (value: string | PromiseLike<string>) => void, chunk: Buffer) {
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
            this.mSshStream!.stderr.removeAllListeners();
            this.mSshStream!.stdout.removeAllListeners();
            this.mResponseStream?.end();
            this.sshMutex?.resolve();
            resolve(this.mResponse);
        }
    }
}
