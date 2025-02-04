// Generated from ds\WriteDataset\Response.schema.json
import type { IRpcResponse } from "../../common/IRpcResponse";

/**
 * Response from writing to a dataset
 *
 * Interface for RPC response objects
 */
export interface Response extends IRpcResponse {
    /**
     * Dataset name that was written to
     */
    dsname: string;
    /**
     * Whether the write operation was successful
     */
    success: boolean;
}
