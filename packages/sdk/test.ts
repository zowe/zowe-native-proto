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
