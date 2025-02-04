// Generated from ds\ListDsMembers\Request.schema.json
import type { IRpcRequest } from "../../common/IRpcRequest";

/**
 * Request to list members of a dataset
 *
 * Interface for RPC request objects
 */
export interface Request extends IRpcRequest {
	command: "listDsMembers";
    /**
     * Dataset name to list members from
     */
    dsname:  string;
    command: string;
}
