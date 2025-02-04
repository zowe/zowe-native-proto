// Generated from ds\RestoreDataset\Request.schema.json
import type { IRpcRequest } from "../../common/IRpcRequest";

/**
 * Request to restore a dataset
 *
 * Interface for RPC request objects
 */
export interface Request extends IRpcRequest {
	command: "restoreDataset";
    /**
     * Dataset name to restore
     */
    dataset: string;
    command: string;
}
