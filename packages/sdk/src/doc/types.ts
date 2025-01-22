export type Dataset = {
    name: string;
    dsorg: string;
    volser: string;
};

export type DsMember = {
    name: string;
};

export type UssItem = {
    name: string;
    isDir: boolean;
};

export type Job = {
    id: string;
    name: string;
    status: string;
    retcode: string;
};

export type Spool = {
    id: number;
    ddname: string;
    stepname: string;
};
