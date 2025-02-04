// Generated from ds\ListDatasets\Request.schema.json
import type { IRpcRequest } from "../../common/IRpcRequest";

/**
 * Request to list datasets matching a pattern
 */
export interface Request extends IRpcRequest {
	command: "listDatasets";
    /**
     * Whether to include dataset attributes
     */
    attributes?: boolean;
    /**
     * Dataset name pattern to match
     */
    pattern: string;
}
