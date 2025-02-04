// Generated from ds\ReadDataset\Response.schema.json
import type { IRpcResponse } from "../../common/IRpcResponse";

/**
 * Response containing dataset contents
 *
 * Interface for RPC response objects
 */
export interface Response extends IRpcResponse {
    data: { [key: string]: any } | string;
    /**
     * Dataset name that was read
     */
    dsname: string;
    /**
     * Encoding used when reading the dataset
     */
    encoding?: string;
    success?:  boolean;
}
