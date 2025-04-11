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

import assert from "node:assert";
import { EventEmitter } from "node:stream";
import { afterEach, before, beforeEach, describe, it, mock } from "node:test";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { Client, type ClientCallback, type ConnectConfig } from "ssh2";
import { ZSshClient } from "../src/ZSshClient";
import { ZSshUtils } from "../src/ZSshUtils";
import type { CommandRequest, RpcRequest, RpcResponse } from "../src/doc";

mock.module("ssh2", require("./mocks/ssh2.mock"));

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

    before(() => {
        mock.timers.enable({ apis: ["setTimeout"] });
    });

    beforeEach(() => {
        mock.method(ZSshUtils, "buildSshConfig", () => ({}));
    });

    afterEach(() => {
        mock.restoreAll();
    });

    describe("create function", () => {
        it("should start SSH client with default options", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession));
            assert(client instanceof ZSshClient);
        });

        it("should handle error thrown by SSH client", async () => {
            const sshError = new Error("bad ssh");
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("error", sshError);
                return this;
            });
            assert.rejects(ZSshClient.create(new SshSession(fakeSession)), sshError);
        });

        it("should handle error in SSH exec callback", async () => {
            const sshError = new Error("bad ssh");
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(Client.prototype, "exec", function (_command: string, callback: ClientCallback) {
                callback(sshError, undefined as any);
                return this;
            });
            assert.rejects(ZSshClient.create(new SshSession(fakeSession)), sshError);
        });

        it("should handle error in SSH data handler", async () => {
            const sshError = new Error("bad ssh");
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(Client.prototype, "exec", function (_command: string, callback: ClientCallback) {
                callback(undefined, sshStream as any);
                sshStream.stderr.emit("data", "");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "onReady", () => {
                throw sshError;
            });
            assert.rejects(ZSshClient.create(new SshSession(fakeSession)), sshError);
        });

        it("should invoke onClose callback", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                this.emit("close");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            const onCloseMock = mock.fn();
            await ZSshClient.create(new SshSession(fakeSession), {
                onClose: onCloseMock,
            });
            assert.equal(onCloseMock.mock.callCount(), 1);
        });

        it("should invoke onError callback", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            const onErrorMock = mock.fn();
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                onError: onErrorMock,
            });
            (client as any).onErrData("bad ssh");
            assert.equal(onErrorMock.mock.callCount(), 1);
        });

        it("should respect keepAliveInterval option", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            const buildSshConfigMock = mock.method(ZSshUtils, "buildSshConfig");
            await ZSshClient.create(new SshSession(fakeSession), {
                keepAliveInterval: 5,
            });
            assert.equal(buildSshConfigMock.mock.callCount(), 1);
            assert.deepEqual(buildSshConfigMock.mock.calls[0].arguments.pop(), { keepaliveInterval: 5e3 });
        });

        it("should respect numWorkers option", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncMock = mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                numWorkers: 1,
            });
            assert.equal(execAsyncMock.mock.callCount(), 1);
            assert.deepEqual(execAsyncMock.mock.calls[0].arguments.slice(1), ["-num-workers", "1"]);
        });

        it("should respect responseTimeout option", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            const client = await ZSshClient.create(new SshSession(fakeSession), {
                responseTimeout: 300,
            });
            assert.equal((client as any).mResponseTimeout, 3e5);
        });

        it("should respect serverPath option", async () => {
            const sshStream = new EventEmitter();
            mock.method(Client.prototype, "connect", function (_config: ConnectConfig) {
                this.emit("ready");
                return this;
            });
            const execAsyncMock = mock.method(ZSshClient.prototype as any, "execAsync", async () => sshStream);
            await ZSshClient.create(new SshSession(fakeSession), {
                serverPath: "/tmp/zowe-server",
            });
            assert.equal(execAsyncMock.mock.callCount(), 1);
            assert.equal(execAsyncMock.mock.calls[0].arguments[0], "/tmp/zowe-server/zowed");
        });
    });

    describe("dispose function", () => {
        it("should end SSH connection", () => {
            const endMock = mock.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client.dispose();
            assert.equal(endMock.mock.callCount(), 1);
        });

        it("should end SSH connection 2", () => {
            const endMock = mock.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = { end: endMock };
            client[Symbol.dispose]();
            assert.equal(endMock.mock.callCount(), 1);
        });
    });

    describe("request function", () => {
        it("should send request that succeeds", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = mock.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).requestEnd(JSON.stringify(rpcResponseGood));
            assert.deepEqual(await response, { success: true });
            assert.deepEqual(writeMock.mock.calls[0].arguments, [`${JSON.stringify(rpcRequest)}\n`]);
        });

        it("should send request that fails", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = mock.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            (client as any).requestEnd(JSON.stringify(rpcResponseBad));
            assert.rejects(response, { message: rpcResponseBad.error?.message });
            assert.deepEqual(writeMock.mock.calls[0].arguments, [`${JSON.stringify(rpcRequest)}\n`]);
        });

        it("should send request that times out", async () => {
            const request: CommandRequest = { command: "ping" };
            const writeMock = mock.fn();
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = { stdin: { write: writeMock } };
            const response = client.request(request);
            mock.timers.runAll();
            assert.rejects(response, { errorCode: "ETIMEDOUT" });
            assert.deepEqual(writeMock.mock.calls[0].arguments, [`${JSON.stringify(rpcRequest)}\n`]);
        });
    });

    describe("callbacks", () => {
        it("should receive ready message from Zowe server", async () => {
            const sshStream = { stderr: new EventEmitter(), stdout: new EventEmitter() };
            const onStderrMock = mock.method(sshStream.stderr, "on");
            const onStdoutMock = mock.method(sshStream.stdout, "on");
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshClient = {
                exec: function (_command: string, callback: ClientCallback) {
                    callback(undefined, sshStream as any);
                    sshStream.stdout.emit("data", readyMessage);
                    return this;
                },
            };
            await (client as any).execAsync();
            assert.equal(onStderrMock.mock.callCount(), 2);
            assert.equal(onStdoutMock.mock.callCount(), 2);
            assert(client.serverChecksums != null);
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
            assert.rejects((client as any).execAsync(), { errorCode: "ENOTFOUND" });
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
            assert.rejects((client as any).execAsync(), "Error starting Zowe server");
        });

        it("should process stderr data from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStderr = new EventEmitter();
            const sshStream = { stdin: { write: mock.fn() }, stdout: { on: mock.fn() }, stderr: fakeStderr };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = sshStream;
            (client as any).onReady(sshStream, readyMessage);
            const response = client.request(request);
            fakeStderr.emit("data", `${JSON.stringify(rpcResponseBad)}\n`);
            assert.rejects(response, { message: rpcResponseBad.error?.message });
        });

        it("should process stdout data from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: mock.fn() }, stdout: fakeStdout, stderr: { on: mock.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mSshStream = sshStream;
            (client as any).onReady(sshStream, readyMessage);
            const response = client.request(request);
            fakeStdout.emit("data", `${JSON.stringify(rpcResponseGood)}\n`);
            assert.deepEqual(await response, { success: true });
        });

        it("should handle invalid response from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: mock.fn() }, stdout: fakeStdout, stderr: { on: mock.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = (err: Error) => {
                throw err;
            };
            (client as any).mSshStream = sshStream;
            (client as any).onReady(sshStream, readyMessage);
            client.request(request);
            assert.throws(() => fakeStdout.emit("data", "bad json\n"), "Invalid JSON response");
        });

        it("should handle unmapped response from Zowe server", async () => {
            const request: CommandRequest = { command: "ping" };
            const fakeStdout = new EventEmitter();
            const sshStream = { stdin: { write: mock.fn() }, stdout: fakeStdout, stderr: { on: mock.fn() } };
            const client: ZSshClient = new (ZSshClient as any)();
            (client as any).mErrHandler = (err: Error) => {
                throw err;
            };
            (client as any).mSshStream = sshStream;
            (client as any).onReady(sshStream, readyMessage);
            client.request(request);
            assert.throws(
                () => fakeStdout.emit("data", `${JSON.stringify({ ...rpcResponseGood, id: -1 })}\n`),
                "Missing promise for response ID",
            );
        });
    });
});
