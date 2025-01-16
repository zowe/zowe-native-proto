import { IProfileLoaded } from "@zowe/imperative";
import { ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";

export class ZSshUtils {
    public static buildSession(profile: IProfileLoaded): SshSession
    {
        const sshSessCfg: ISshSession = {
            hostname: profile.profile!.host,
            port: profile.profile?.port ?? 22,
            user: profile.profile!.user,
            privateKey: profile.profile?.privateKey,
            keyPassphrase: profile.profile?.privateKey ? profile.profile?.keypassPhrase : undefined,
            password: profile.profile?.privateKey ? undefined : profile.profile?.password,
        };

        return new SshSession(sshSessCfg);
    }

    public static deployServer(): void {}
}
