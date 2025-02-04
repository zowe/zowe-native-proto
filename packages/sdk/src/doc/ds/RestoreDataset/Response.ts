// Generated from ds\RestoreDataset\Response.schema.json
import type { IRpcResponse } from "../../common/IRpcResponse";

/**
 * Response from restoring a dataset
 *
 * Interface for RPC response objects
 */
export interface Response extends IRpcResponse {
    /**
     * Whether the restore operation was successful
     */
    success: boolean;
}
