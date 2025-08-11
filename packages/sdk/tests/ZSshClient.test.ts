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

import { EventEmitter, Readable } from "node:stream";
import { Logger } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientCallback, type ConnectConfig } from "ssh2";
import type { CommandRequest, RpcRequest, RpcResponse } from "../src/doc";
import { ZSshClient } from "../src/ZSshClient";
import { ZSshUtils } from "../src/ZSshUtils";

vi.mock("ssh2");

describe("ZSshClient", () => {
    const fakeSession: ISshSession = {
        hostname: "example.com",
        port: 22,
        user: "admin",
        privateKey: "~/.ssh/id_rsa",
    };
    const readyMessage = JSON.stringify({ status: "ready", data: { checksums: "sha256" } });
    const rpcRequest: RpcRequest = {
        jsonrpc: "2.0",
        method: "ping",
        params: {},
        id: 1,
    };
    const rpcResponseBad: RpcResponse = {
        jsonrpc: "2.0",
        error: { code: 0, message: "bad rpc" },
        id: 1,
    };
    const rpcResponseGood: RpcResponse = {
        jsonrpc: "2.0",
        result: { success: true },
        id: 1,
    };

    beforeAll(() => {
        vi.useFakeTimers();
    });

    beforeEach(() => {
        vi.spyOn(ZSshUtils, "buildSshConfig").mockReturnValue({});
    });

    describe("create function", () => {
        it("should start SSH client with default options", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession));
            expect(client).toBeInstanceOf(ZSshClient);
        });

        it("should handle error thrown by SSH client", async () => {
            const sshError = new Error("bad ssh");
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("error", sshError);
                return this;
            });
            await expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should handle error in SSH exec callback", async () => {
            const sshError = new Error("bad ssh");
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(Client.prototype, "exec").mockImplementation(function (
                _command: string,
                callback: ClientCallback,
            ) {
                callback(sshError, undefined as any);
                return this;
            });
            await expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should handle error in SSH data handler", async () => {
            const sshError = new Error("bad ssh");
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(Client.prototype, "exec").mockImplementation(function (
                _command: string,
                callback: ClientCallback,
            ) {
                callback(undefined, sshStream as any);
                sshStream.stderr.emit("data", "");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "getServerStatus").mockImplementation(() => {
                throw sshError;
            });
            await expect(ZSshClient.create(new SshSession(fakeSession))).rejects.toThrow(sshError);
        });

        it("should invoke onClose callback", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                this.emit("close");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            const onCloseMock = vi.fn();
            await ZSshClient.create(new SshSession(fakeSession), {
                onClose: onCloseMock,
            });
            expect(onCloseMock).toHaveBeenCalledTimes(1);
        });

        it("should invoke onError callback", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            const onErrorMock = vi.fn();
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                onError: onErrorMock,
            });
            (client as any).onErrData("bad ssh");
            expect(onErrorMock).toHaveBeenCalledTimes(1);
        });

        it("should respect keepAliveInterval option", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            const buildSshConfigMock = vi.spyOn(ZSshUtils, "buildSshConfig");
            await ZSshClient.create(new SshSession(fakeSession), {
                keepAliveInterval: 5,
            });
            expect(buildSshConfigMock).toHaveBeenCalledTimes(1);
            expect(buildSshConfigMock.mock.calls[0][buildSshConfigMock.mock.calls[0].length - 1]).toEqual({
                keepaliveInterval: 5e3,
            });
        });

        it("should respect numWorkers option", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncMock = vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                numWorkers: 1,
            });
            expect(execAsyncMock).toHaveBeenCalledTimes(1);
            expect(execAsyncMock.mock.calls[0].slice(1)).toEqual(["-num-workers", "1"]);
        });

        it("should respect responseTimeout option", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                responseTimeout: 300,
            });
            expect((client as any).mResponseTimeout).toBe(3e5);
        });

        it("should respect serverPath option", async () => {
            const sshStream = new EventEmitter();
            vi.spyOn(Client.prototype, "connect").mockImplementation(function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncMock = vi.spyOn(ZSshClient.prototype as any, "execAsync").mockResolvedValue(sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                serverPath: "/tmp/zowe-server",
            });
            expect(execAsyncMock).toHaveBeenCalledTimes(1);
            expect(execAsyncMock.mock.calls[0][0]).toBe("/tmp/zowe-server/zowed");
        });
    });

    describe("dispose function", () => {
        it("should end SSH connection", () => {
            const endMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client.dispose();
            expect(endMock).toHaveBeenCalledTimes(1);
        });

        it("should end SSH connection 2", () => {
            const endMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client[Symbol.dispose]();
            expect(endMock).toHaveBeenCalledTimes(1);
        });
    });

    describe("request function", () => {
        it("should send request that succeeds", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).processResponses(`${JSON.stringify(rpcResponseGood)}\n`);
            expect(await response).toEqual({ success: true });
            expect(writeMock.mock.calls[0]).toEqual([`${JSON.stringify(rpcRequest)}\n`]);
        });

        it("should send request that fails", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).processResponses(`${JSON.stringify(rpcResponseBad)}\n`);
            await expect(response).rejects.toMatchObject({ message: rpcResponseBad.error?.message });
            expect(writeMock.mock.calls[0]).toEqual([`${JSON.stringify(rpcRequest)}\n`]);
        });

        it("should send request that times out", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            vi.runAllTimers();
            await expect(response).rejects.toMatchObject({ errorCode: "ETIMEDOUT" });
            expect(writeMock.mock.calls[0]).toEqual([`${JSON.stringify(rpcRequest)}\n`]);
        });

        it("should skip empty response lines", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);

            const response = client.request(request);
            // Send response with empty lines
            fakeStdout.emit("data", `\n\n${JSON.stringify(rpcResponseGood)}\n`);

            expect(await response).toEqual({ success: true });
        });
    });

    describe("callbacks", () => {
        it("should receive ready message from Zowe server", async () => {
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            const onStderrMock = vi.spyOn(sshStream.stderr, "on");
            const onStdoutMock = vi.spyOn(sshStream.stdout, "on");
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = {
                exec: function (_command: string, callback: ClientCallback) {
                    callback(undefined, sshStream as any);
                    sshStream.stdout.emit("data", readyMessage);
                    return this;
                },
            };
            await (client as any).execAsync();
            expect(onStderrMock).toHaveBeenCalledTimes(2);
            expect(onStdoutMock).toHaveBeenCalledTimes(2);
            expect(client.serverChecksums).not.toBeNull();
        });

        it("should handle not found error from Zowe server", async () => {
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = {
                exec: function (_command: string, callback: ClientCallback) {
                    callback(undefined, sshStream as any);
                    sshStream.stderr.emit("data", "FSUM7351 not found");
                    return this;
                },
            };
            await expect((client as any).execAsync()).rejects.toMatchObject({ errorCode: "ENOTFOUND" });
        });

        it("should handle startup error from Zowe server", async () => {
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = {
                exec: function (_command: string, callback: ClientCallback) {
                    callback(undefined, sshStream as any);
                    sshStream.stderr.emit("data", "bad json");
                    return this;
                },
            };
            await expect((client as any).execAsync()).rejects.toThrow("Error starting Zowe server");
        });

        it("should process stderr data from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStderr = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: { on: vi.fn() }, stderr: fakeStderr };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            const response = client.request(request);
            fakeStderr.emit("data", `${JSON.stringify(rpcResponseBad)}\n`);
            await expect(response).rejects.toMatchObject({ message: rpcResponseBad.error?.message });
        });

        it("should process stdout data from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            const response = client.request(request);
            fakeStdout.emit("data", `${JSON.stringify(rpcResponseGood)}\n`);
            expect(await response).toEqual({ success: true });
        });

        it("should handle chdir error from Zowe server without throwing", async () => {
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            const onErrorMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = onErrorMock;
            (client as any).mSshClient = {
                exec: function (_command: string, callback: ClientCallback) {
                    callback(undefined, sshStream as any);
                    sshStream.stderr.emit("data", "FOTS1681 chdir error");
                    // Send ready message after the non-fatal error
                    sshStream.stdout.emit("data", readyMessage);
                    return this;
                },
            };

            await (client as any).execAsync();
            expect(onErrorMock).toHaveBeenCalledTimes(1);
            expect(onErrorMock.mock.calls[0][0]).toBeInstanceOf(Error);
        });

        it("should handle invalid response from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = (err: Error) => {
                throw err;
            };
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            client.request(request);
            expect(() => fakeStdout.emit("data", "bad json\n")).toThrow("Invalid JSON response");
        });

        it("should handle unmapped response from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = (err: Error) => {
                throw err;
            };
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            client.request(request);
            expect(() => fakeStdout.emit("data", `${JSON.stringify({ ...rpcResponseGood, id: -1 })}\n`)).toThrow(
                "Missing promise for response ID",
            );
        });

        it("should log message with default error handler", () => {
            const logErrorMock = vi.fn();
            vi.spyOn(Logger, "getAppLogger").mockReturnValue({
                error: logErrorMock,
            } as any);
            const testError = new Error("test error");
            (ZSshClient as any).defaultErrHandler(testError);
            expect(logErrorMock).toHaveBeenCalledTimes(1);
            expect(logErrorMock.mock.calls[0][0]).toBe(`Error: ${testError.message}`);
        });
    });

    describe("request with stream", () => {
        it("should register stream when request contains stream", async () => {
            const mockStream = new Readable({ read() {} });
            const request: CommandRequest & { stream: Readable } = {
                command: "ping",
                stream: mockStream,
            };
            const writeMock = vi.fn();
            const registerStreamMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            (client as any).mNotifMgr = { registerStream: registerStreamMock };

            const response = client.request(request);
            (client as any).processResponses(`${JSON.stringify(rpcResponseGood)}\n`);
            await response;

            expect(registerStreamMock).toHaveBeenCalledTimes(1);
            expect(registerStreamMock.mock.calls[0][1]).toBe(mockStream);
        });
    });

    describe("notification handling", () => {
        it("should handle RPC notifications", async () => {
            const notification = {
                jsonrpc: "2.0",
                method: "sendStream",
                params: { id: 1, pipePath: "/dev/null" },
            };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            const handleNotificationMock = vi.fn();
            (client as any).mNotifMgr = { handleNotification: handleNotificationMock };
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            (client as any).mPromiseMap.set(1, { resolve: vi.fn(), reject: vi.fn() });

            fakeStdout.emit("data", `${JSON.stringify(notification)}\n`);

            expect(handleNotificationMock).toHaveBeenCalledTimes(1);
            expect(handleNotificationMock.mock.calls[0][0]).toEqual(notification);
        });

        it("should handle errors in notification processing", async () => {
            const notification = {
                jsonrpc: "2.0",
                method: "sendStream",
                params: { id: 1, pipePath: "/dev/null" },
            };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: vi.fn() }, stdout: fakeStdout, stderr: { on: vi.fn() } };
            const onErrorMock = vi.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = onErrorMock;
            (client as any).mNotifMgr = {
                handleNotification: () => {
                    throw new Error("notification error");
                },
            };
            (client as any).mSshStream = sshStream;
            (client as any).getServerStatus(sshStream, readyMessage);
            (client as any).mPromiseMap.set(1, { resolve: vi.fn(), reject: vi.fn() });

            fakeStdout.emit("data", `${JSON.stringify(notification)}\n`);

            expect(onErrorMock).toHaveBeenCalledTimes(1);
            expect(onErrorMock.mock.calls[0][0]).toBeInstanceOf(Error);
        });
    });
});
