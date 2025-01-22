import type { IProfile } from "@zowe/imperative";
import { type ISshSession, SshSession } from "@zowe/zos-uss-for-zowe-sdk";

// biome-ignore lint/complexity/noStaticOnlyClass: <explanation>
export class ZSshUtils {
    public static buildSession(args: IProfile): SshSession {
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

    public static decodeByteArray(data: Buffer | string): Buffer {
        return typeof data === "string" ? Buffer.from(data, "base64") : data;
    }

    public static encodeByteArray(data: Buffer | string): string {
        return typeof data !== "string" ? Buffer.from(data).toString("base64") : data;
    }

    public static deployServer(): void {}
}
