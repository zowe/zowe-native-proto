// Generated from ds\ListDatasets\Response.schema.json
import type { IRpcResponse } from "../../common/IRpcResponse";

/**
 * Response containing list of datasets
 *
 * Interface for RPC response objects
 */
export interface Response extends IRpcResponse {
    items: Dataset[];
    /**
     * Number of datasets returned
     */
    returnedRows: number;
    success?:     boolean;
}

export interface Dataset {
    dsorg:  string;
    name:   string;
    volser: string;
    [property: string]: any;
}
