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

import Module from "node:module";
import { vi } from "vitest";
import { createVSCodeMock } from "jest-mock-vscode";

// Intercept vscode require for CommonJS modules in node_modules that call require("vscode") directly.
// This module must be imported before any module that transitively requires vscode.
const vscodeMock = createVSCodeMock(vi);
const originalRequire = Module.prototype.require;
(Module.prototype as any).require = function (id: string) {
    if (id === "vscode") return vscodeMock;
    return originalRequire.apply(this, arguments as any);
};

vi.mock("vscode", () => vscodeMock, { virtual: true });

export interface BenchmarkTarget {
    name: string;
    mvs: any;
    uss: any;
    jes: any;
    dsName: string;
    ussFile: string;
    job?: { jobid: string; jobname: string };
}

export const RANDOM_STR = Math.random().toString(36).substring(2, 8).toUpperCase();
export const USS_DIR = `/tmp`;
export const DUMMY_JCL = `//BENCHJOB JOB (ACCT),'BENCHMARK',CLASS=A,MSGCLASS=X,MSGLEVEL=(1,1)\n//STEP1    EXEC PGM=IEFBR14\n`;
export const DS_OPTS = { alcunit: "TRK", primary: 10, secondary: 5 };

export let PREFIX = process.env.ZOWE_PREFIX || "";

export const targets: BenchmarkTarget[] = [
    { name: "z/OSMF", mvs: null, uss: null, jes: null, dsName: "", ussFile: "" },
    { name: "SSH",    mvs: null, uss: null, jes: null, dsName: "", ussFile: "" },
];

export async function setupTargets(): Promise<void> {
    const explorerApi = await import("@zowe/zowe-explorer-api");
    const ZoweExplorerZosmf = explorerApi.ZoweExplorerZosmf;
    const imperative = explorerApi.imperative;

    const SshMvsApi = (await import("../src/api/SshMvsApi")).SshMvsApi;
    const SshUssApi = (await import("../src/api/SshUssApi")).SshUssApi;
    const SshJesApi = (await import("../src/api/SshJesApi")).SshJesApi;

    const profileInfo = new imperative.ProfileInfo("zowe");
    await profileInfo.readProfilesFromDisk();

    const zosmfAttrs = profileInfo.getDefaultProfile("zosmf");
    const sshAttrs = profileInfo.getDefaultProfile("ssh");
    if (!zosmfAttrs || !sshAttrs) throw new Error("Default zosmf or ssh profile not found");

    const toProfile = (merged: any) =>
        Object.fromEntries(merged.knownArgs.map((arg: any) => [arg.argName, arg.argValue]));

    const zosmfProfile = toProfile(profileInfo.mergeArgsForProfile(zosmfAttrs, { getSecureVals: true }));
    const sshProfile   = toProfile(profileInfo.mergeArgsForProfile(sshAttrs,   { getSecureVals: true }));

    if (!PREFIX) PREFIX = ((zosmfProfile.user as string) ?? "USER").toUpperCase();

    targets[0].dsName  = `${PREFIX}.BZ${RANDOM_STR}`;
    targets[0].ussFile = `${USS_DIR}/bench-zosmf-${RANDOM_STR}.txt`;
    targets[1].dsName  = `${PREFIX}.BS${RANDOM_STR}`;
    targets[1].ussFile = `${USS_DIR}/bench-ssh-${RANDOM_STR}.txt`;

    const zosmfLoaded = { name: zosmfAttrs.name, type: zosmfAttrs.type, profile: zosmfProfile, failNotFound: false };
    const sshLoaded   = { name: sshAttrs.name,   type: sshAttrs.type,   profile: sshProfile,   failNotFound: false };

    targets[0].mvs = new ZoweExplorerZosmf.MvsApi(zosmfLoaded);
    targets[0].uss = new ZoweExplorerZosmf.UssApi(zosmfLoaded);
    targets[0].jes = new ZoweExplorerZosmf.JesApi(zosmfLoaded);

    targets[1].mvs = new SshMvsApi(undefined, sshLoaded);
    targets[1].uss = new SshUssApi(sshLoaded);
    targets[1].jes = new SshJesApi(sshLoaded);
}
