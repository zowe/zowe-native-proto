/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import { EventEmitter } from "node:stream";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientCallback, type ConnectConfig } from "ssh2";
import { ZSshClient } from "../src/ZSshClient";
import { ZSshUtils } from "../src/ZSshUtils";
import type { CommandRequest, RpcRequest, RpcResponse } from "../src/doc";

jest.mock("ssh2");

describe("ZSshClient", () => {
    const fakeSession: ISshSession = {
        hostname: "example.com",
        port: 22,
        user: "admin",
        privateKey: "~/.ssh/id_rsa",
    };
    const readyMessage = JSON.stringify({ status: "ready" });
    const rpcRequest: RpcRequest = {
        jsonrpc: "2.0",
        method: "ping",
        params: {},
        id: 1,
    };
    const rpcResponseBad: RpcResponse = {
        jsonrpc: "2.0",
        error: {
            code: 0,
            message: "bad rpc",
        },
        id: 1,
    };
    const rpcResponseGood: RpcResponse = {
        jsonrpc: "2.0",
        result: {},
        id: 1,
    };

    beforeAll(() => {
        jest.useFakeTimers();
        jest.spyOn(ZSshUtils, "buildSshConfig").mockReturnValue({});
    });

    afterEach(() => {
        jest.clearAllMocks();
    });

    describe("create", () => {
        it("should start SSH client with default options", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession));
            expect(client).toBeInstanceOf(ZSshClient);
        });

        it("should handle error thrown by SSH client", async () => {
            const sshError = new Error("bad ssh");
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("error", sshError);
                return this;
            });
            expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should handle error in SSH exec callback", async () => {
            const sshError = new Error("bad ssh");
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(Client.prototype, "exec").mockImplementationOnce(function (
                _command: string,
                callback: ClientCallback,
            ) {
                callback(sshError, undefined);
                return this;
            });
            expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should handle error in SSH data handler", async () => {
            const sshError = new Error("bad ssh");
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(Client.prototype, "exec").mockImplementationOnce(function (
                _command: string,
                callback: ClientCallback,
            ) {
                callback(undefined, sshStream as any);
                sshStream.emit("data");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "onReady").mockRejectedValueOnce(sshError);
            expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should invoke onClose callback", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                this.emit("close");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            const onCloseMock = jest.fn();
            await ZSshClient.create(new SshSession(fakeSession), {
                onClose: onCloseMock,
            });
            expect(onCloseMock).toHaveBeenCalledTimes(1);
        });

        it("should invoke onConnect callback", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            const onConnectMock = jest.fn();
            await ZSshClient.create(new SshSession(fakeSession), {
                onConnect: onConnectMock,
            });
            expect(onConnectMock).toHaveBeenCalledTimes(1);
        });

        it("should invoke onError callback", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            const onErrorMock = jest.fn();
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                onError: onErrorMock,
            });
            (client as any).onErrData("bad ssh");
            expect(onErrorMock).toHaveBeenCalledTimes(1);
        });

        it("should respect numWorkers option", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncSpy = jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                numWorkers: 1,
            });
            expect(execAsyncSpy).toHaveBeenCalledTimes(1);
            expect(execAsyncSpy.mock.calls[0]).toEqual(expect.arrayContaining(["-num-workers", "1"]));
        });

        it("should respect responseTimeout option", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                responseTimeout: 300,
            });
            expect((client as any).mResponseTimeout).toBe(3e5);
        });

        it("should respect serverPath option", async () => {
            const sshStream = new EventEmitter();
            jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncSpy = jest.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValueOnce(sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                serverPath: "/tmp/zowe-server",
            });
            expect(execAsyncSpy).toHaveBeenCalledTimes(1);
            expect(execAsyncSpy.mock.calls[0]).toContain("/tmp/zowe-server/zowed");
        });
    });

    describe("dispose", () => {
        it("should end SSH connection", () => {
            const endMock = jest.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client.dispose();
            expect(endMock).toHaveBeenCalledTimes(1);
        });

        it("should end SSH connection 2", () => {
            const endMock = jest.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client[Symbol.dispose]();
            expect(endMock).toHaveBeenCalledTimes(1);
        });
    });

    describe("request", () => {
        it("should send request that succeeds", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = jest.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).requestEnd(JSON.stringify(rpcResponseGood));
            expect(await response).toEqual({ success: true });
            expect(writeMock).toHaveBeenCalledWith(JSON.stringify(rpcRequest) + "\n");
        });

        it("should send request that fails", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = jest.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).requestEnd(JSON.stringify(rpcResponseBad));
            let caughtError: Error | undefined;
            try {
                await response;
            } catch (err) {
                caughtError = err;
            }
            expect(caughtError).toBeDefined();
            expect(caughtError?.message).toBe(rpcResponseBad.error?.message);
            expect(writeMock).toHaveBeenCalledWith(JSON.stringify(rpcRequest) + "\n");
        });

        it("should send request that times out", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = jest.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            jest.advanceTimersToNextTimer();
            let caughtError: (Error & { errorCode?: string }) | undefined;
            try {
                await response;
            } catch (err) {
                caughtError = err;
            }
            expect(caughtError).toBeDefined();
            expect(caughtError?.errorCode).toBe("ETIMEDOUT");
            expect(writeMock).toHaveBeenCalledWith(JSON.stringify(rpcRequest) + "\n");
        });
    });

    it("should receive ready message from Zowe server", async () => {
        const sshStream = new EventEmitter();
        const onStderrMock = jest.fn();
        const onStdoutMock = jest.fn();
        Object.defineProperty(sshStream, "stderr", { value: { on: onStderrMock } });
        Object.defineProperty(sshStream, "stdout", { value: { on: onStdoutMock } });
        jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
            this.emit("ready");
            return this;
        });
        jest.spyOn(Client.prototype, "exec").mockImplementationOnce(function (
            _command: string,
            callback: ClientCallback,
        ) {
            callback(undefined, sshStream as any);
            sshStream.emit("data", readyMessage);
            return this;
        });
        await ZSshClient.create(new SshSession(fakeSession));
        expect(onStderrMock).toHaveBeenCalledTimes(1);
        expect(onStdoutMock).toHaveBeenCalledTimes(1);
    });

    it("should handle not found error from Zowe server", async () => {
        const sshStream = new EventEmitter();
        jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
            this.emit("ready");
            return this;
        });
        jest.spyOn(Client.prototype, "exec").mockImplementationOnce(function (
            _command: string,
            callback: ClientCallback,
        ) {
            callback(undefined, sshStream as any);
            sshStream.emit("data", "FSUM7351 not found");
            return this;
        });
        let caughtError: (Error & { errorCode?: string }) | undefined;
        try {
            await ZSshClient.create(new SshSession(fakeSession));
        } catch (err) {
            caughtError = err;
        }
        expect(caughtError).toBeDefined();
        expect(caughtError?.errorCode).toBe("ENOTFOUND");
    });

    it("should handle unknown error from Zowe server", async () => {
        const sshStream = new EventEmitter();
        jest.spyOn(Client.prototype, "connect").mockImplementationOnce(function (_config: ConnectConfig) {
            this.emit("ready");
            return this;
        });
        jest.spyOn(Client.prototype, "exec").mockImplementationOnce(function (
            _command: string,
            callback: ClientCallback,
        ) {
            callback(undefined, sshStream as any);
            sshStream.emit("data", "bad json");
            return this;
        });
        let caughtError: Error | undefined;
        try {
            await ZSshClient.create(new SshSession(fakeSession));
        } catch (err) {
            caughtError = err;
        }
        expect(caughtError).toBeDefined();
        expect(caughtError?.message).toContain("Error starting Zowe server");
    });

    it("should process stderr data from Zowe server", async () => {
        const request: CommandRequest = { command: "ping" };
        const fakeStderr = new EventEmitter();
        const sshStream = { stdin: { write: jest.fn() }, stdout: { on: jest.fn() }, stderr: fakeStderr };
        const client: ZSshClient = new (ZSshClient as any)();
        (client as any).mSshStream = (client as any).onReady(sshStream, readyMessage);
        const response = client.request(request);
        fakeStderr.emit("data", JSON.stringify(rpcResponseBad) + "\n");
        let caughtError: Error | undefined;
        try {
            await response;
        } catch (err) {
            caughtError = err;
        }
        expect(caughtError).toBeDefined();
        expect(caughtError?.message).toBe(rpcResponseBad.error?.message);
    });

    it("should process stdout data from Zowe server", async () => {
        const request: CommandRequest = { command: "ping" };
        const fakeStdout = new EventEmitter();
        const sshStream = { stdin: { write: jest.fn() }, stdout: fakeStdout, stderr: { on: jest.fn() } };
        const client: ZSshClient = new (ZSshClient as any)();
        (client as any).mSshStream = (client as any).onReady(sshStream, readyMessage);
        const response = client.request(request);
        fakeStdout.emit("data", JSON.stringify(rpcResponseGood) + "\n");
        expect(await response).toEqual({ success: true });
    });

    it("should handle invalid response from Zowe server", async () => {
        const request: CommandRequest = { command: "ping" };
        const fakeStdout = new EventEmitter();
        const sshStream = { stdin: { write: jest.fn() }, stdout: fakeStdout, stderr: { on: jest.fn() } };
        const client: ZSshClient = new (ZSshClient as any)();
        (client as any).mErrHandler = (err: Error) => {
            throw err;
        };
        (client as any).mSshStream = (client as any).onReady(sshStream, readyMessage);
        client.request(request);
        let caughtError: Error | undefined;
        try {
            fakeStdout.emit("data", "bad json\n");
        } catch (err) {
            caughtError = err;
        }
        expect(caughtError).toBeDefined();
        expect(caughtError?.message).toContain("Invalid JSON response");
    });

    it("should handle unmapped response from Zowe server", async () => {
        const request: CommandRequest = { command: "ping" };
        const fakeStdout = new EventEmitter();
        const sshStream = { stdin: { write: jest.fn() }, stdout: fakeStdout, stderr: { on: jest.fn() } };
        const client: ZSshClient = new (ZSshClient as any)();
        (client as any).mErrHandler = (err: Error) => {
            throw err;
        };
        (client as any).mSshStream = (client as any).onReady(sshStream, readyMessage);
        client.request(request);
        let caughtError: Error | undefined;
        try {
            fakeStdout.emit("data", JSON.stringify({ ...rpcResponseGood, id: -1 }) + "\n");
        } catch (err) {
            caughtError = err;
        }
        expect(caughtError).toBeDefined();
        expect(caughtError?.message).toContain("Missing promise for response ID");
    });
});
