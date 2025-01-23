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

import { ProfileInfo } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { ZSshClient } from "./src";

(async () => {
    const profInfo = new ProfileInfo("zowe");
    await profInfo.readProfilesFromDisk();
    const sshProfAttrs = profInfo.getDefaultProfile("ssh");
    const sshMergedArgs = profInfo.mergeArgsForProfile(sshProfAttrs, { getSecureVals: true });
    const session = new SshSession(ProfileInfo.initSessCfg(sshMergedArgs.knownArgs));
    using client = await ZSshClient.create(session);
    for (const fspath of ["/u/users/timothy", "/u/users/trae"]) {
        console.time(`listFiles:${fspath}`);
        const response = await client.uss.listFiles({ fspath });
        console.timeEnd(`listFiles:${fspath}`);
        console.log(response.items.map((item) => item.name));
    }
})().catch((err) => {
    console.error(err);
    process.exit(1);
});
