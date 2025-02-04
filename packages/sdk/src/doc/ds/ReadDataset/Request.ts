// Generated from ds\ReadDataset\Request.schema.json
import type { IRpcRequest } from "../../common/IRpcRequest";

/**
 * Request to read contents of a dataset
 *
 * Interface for RPC request objects
 */
export interface Request extends IRpcRequest {
	command: "readDataset";
    /**
     * Dataset name to read
     */
    dsname: string;
    /**
     * Encoding to use when reading the dataset
     */
    encoding?: string;
    command:   string;
}
