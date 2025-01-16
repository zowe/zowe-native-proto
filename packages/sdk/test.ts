import { ProfileInfo } from "@zowe/imperative";
import { SshSession } from "@zowe/zos-uss-for-zowe-sdk";
import { IListUssRequest, IListUssResponse, ZSshClient } from "./src";

(async () => {
    const profInfo = new ProfileInfo("zowe");
    await profInfo.readProfilesFromDisk();
    const sshProfAttrs = profInfo.getDefaultProfile("ssh");
    const sshMergedArgs = profInfo.mergeArgsForProfile(sshProfAttrs, { getSecureVals: true });
    const session = new SshSession(ProfileInfo.initSessCfg(sshMergedArgs.knownArgs));
    for (const fspath of ["/u/users/timothy", "/u/users/trae"]) {
        const request: IListUssRequest = { command: "listUss", fspath };
        console.time(`${request.command}:${request.fspath}`);
        const response = await ZSshClient.inst.request<IListUssResponse>(session, request);
        console.timeEnd(`${request.command}:${request.fspath}`);
        console.log(response.items.map((item) => item.name));
    }
    ZSshClient.inst.dispose();
})().catch((err) => {
    console.error(err);
    process.exit(1);
});
