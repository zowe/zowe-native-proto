// Generated from ds\WriteDataset\Request.schema.json
import type { IRpcRequest } from "../../common/IRpcRequest";

/**
 * Request to write contents to a dataset
 *
 * Interface for RPC request objects
 */
export interface Request extends IRpcRequest {
	command: "writeDataset";
    contents: { [key: string]: any } | string;
    /**
     * Dataset name to write to
     */
    dsname: string;
    /**
     * Encoding to use when writing the dataset
     */
    encoding?: string;
    command:   string;
}
