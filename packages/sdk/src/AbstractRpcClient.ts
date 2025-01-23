import type { IRpcRequest, IRpcResponse } from "./doc";
import type * as ds from "./doc/zos-ds";
import type * as jobs from "./doc/zos-jobs";
import type * as uss from "./doc/zos-uss";

export abstract class AbstractRpcClient {
    public abstract request<T extends IRpcResponse>(request: IRpcRequest): Promise<T>;

    public get ds() {
        return {
            listDatasets: (request: Omit<ds.ListDatasets.Request, "command">): Promise<ds.ListDatasets.Response> =>
                this.request({ command: "listDatasets", ...request }),
            listDsMembers: (request: Omit<ds.ListDsMembers.Request, "command">): Promise<ds.ListDsMembers.Response> =>
                this.request({ command: "listDsMembers", ...request }),
            readDataset: (request: Omit<ds.ReadDataset.Request, "command">): Promise<ds.ReadDataset.Response> =>
                this.request({ command: "readDataset", ...request }),
            writeDataset: (request: Omit<ds.WriteDataset.Request, "command">): Promise<ds.WriteDataset.Response> =>
                this.request({ command: "writeDataset", ...request }),
        };
    }

    public get jobs() {
        return {
            getJcl: (request: Omit<jobs.GetJcl.Request, "command">): Promise<jobs.GetJcl.Response> =>
                this.request({ command: "getJcl", ...request }),
            listJobs: (request: Omit<jobs.ListJobs.Request, "command">): Promise<jobs.ListJobs.Response> =>
                this.request({ command: "listJobs", ...request }),
            listSpools: (request: Omit<jobs.ListSpools.Request, "command">): Promise<jobs.ListSpools.Response> =>
                this.request({ command: "listSpools", ...request }),
            readSpool: (request: Omit<jobs.ReadSpool.Request, "command">): Promise<jobs.ReadSpool.Response> =>
                this.request({ command: "readSpool", ...request }),
        };
    }

    public get uss() {
        return {
            listFiles: (request: Omit<uss.ListFiles.Request, "command">): Promise<uss.ListFiles.Response> =>
                this.request({ command: "listFiles", ...request }),
            readFile: (request: Omit<uss.ReadFile.Request, "command">): Promise<uss.ReadFile.Response> =>
                this.request({ command: "readFile", ...request }),
            writeFile: (request: Omit<uss.WriteFile.Request, "command">): Promise<uss.WriteFile.Response> =>
                this.request({ command: "writeFile", ...request }),
        };
    }
}
