import { ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";

export class ZSshUtils {
    public static buildSession(args: Record<string, any>): SshSession
    {
        const sshSessCfg: ISshSession = {
            hostname: args.host,
            port: args.port ?? 22,
            user: args.user,
            privateKey: args.privateKey,
            keyPassphrase: args.privateKey ? args.keyPassphrase : undefined,
            password: args.privateKey ? undefined : args.password,
        };

        return new SshSession(sshSessCfg);
    }

    public static deployServer(): void {}
}
