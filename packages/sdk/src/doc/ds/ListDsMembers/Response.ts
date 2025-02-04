// Generated from ds\ListDsMembers\Response.schema.json
import type { IRpcResponse } from "../../common/IRpcResponse";

/**
 * Response containing list of dataset members
 *
 * Interface for RPC response objects
 */
export interface Response extends IRpcResponse {
    items: Item[];
    /**
     * Number of members returned
     */
    returnedRows: number;
    success?:     boolean;
}

export interface Item {
    name: string;
    [property: string]: any;
}
