# API Matrix

### Legend

- ✅ Supported
- 🚧 Partially supported
- ❌ Not supported
- ➖ Not applicable
- _italic_ Target for MVP

## Data Sets

| Operation                | z/OSMF          | FTP | Backend         | Middleware | SDK | CLI | VSCE |
| ------------------------ | --------------- | --- | --------------- | ---------- | --- | --- | ---- |
| _List data sets_         | ✅              | ✅  | 🚧 <sup>1</sup> | ✅         | ✅  | ✅  | ✅   |
| _List data set members_  | ✅              | ✅  | 🚧 <sup>1</sup> | ✅         | ✅  | ✅  | ✅   |
| _Read data set_          | ✅              | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ✅  | ✅   |
| _Read data set member_   | ✅              | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ✅  | ✅   |
| _Write data set_         | ✅              | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ❌  | ❌   |
| _Write data set member_  | ✅              | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ❌  | ❌   |
| _Create data set_        | ✅              | ✅  | 🚧 <sup>3</sup> | ❌         | ❌  | ❌  | ❌   |
| _Create data set member_ | ✅              | ✅  | ❌              | ❌         | ❌  | ❌  | ❌   |
| _Delete data set_        | ✅              | ✅  | ✅              | ❌         | ❌  | ❌  | ❌   |
| _Delete data set member_ | ✅              | ✅  | ✅              | ❌         | ❌  | ❌  | ❌   |
| Migrate data set         | ✅              | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Recall data set          | 🚧 <sup>4</sup> | ❌  | ✅              | ✅         | ✅  | ✅  | ❌   |
| Delete migrated data set | ✅              | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Rename data set          | ✅              | ✅  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Copy data set            | ✅              | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Invoke AMS (VSAM)        | ✅              | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Search data sets         | 🚧 <sup>5</sup> | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |

1. Not all attributes are retrieved
2. Streaming is not supported for large files
3. Does not support allocation attributes
4. Does not support some migration utilities like CA Disk
5. Limited options compared to ISPF `srchfor`

## USS Files

| Operation                | z/OSMF | FTP | Backend         | Middleware | SDK | CLI | VSCE |
| ------------------------ | ------ | --- | --------------- | ---------- | --- | --- | ---- |
| _List files/directories_ | ✅     | ✅  | 🚧 <sup>1</sup> | ✅         | ✅  | ✅  | ✅   |
| _Read USS file_          | ✅     | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ✅  | ❌   |
| _Write USS file_         | ✅     | ✅  | 🚧 <sup>2</sup> | ✅         | ✅  | ❌  | ❌   |
| _Create file/directory_  | ✅     | ✅  | 🚧 <sup>3</sup> | ✅         | ❌  | ❌  | ❌   |
| _Delete file/directory_  | ✅     | ✅  | ✅              | ❌         | ❌  | ❌  | ❌   |
| Copy file/directory      | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Move file/directory      | ✅     | ✅  | ❌              | ❌         | ❌  | ❌  | ❌   |
| _`chmod` file/directory_ | ✅     | ❌  | ✅              | ✅         | ✅  | ✅  | ❌   |
| _`chown` file/directory_ | ✅     | ❌  | ✅              | ❌         | ❌  | ❌  | ❌   |
| _`chtag` file/directory_ | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Invoke `extattr`         | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |
| Get ACL entries          | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |
| Set ACL entries          | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |
| Link file/directory      | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |
| Unlink file/directory    | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |
| List z/OS file systems   | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Create z/OS file system  | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Delete z/OS file system  | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Mount file system        | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Unmount file system      | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |

1. Not all attributes are retrieved
2. Streaming is not supported for large files
3. Recursive option is not supported

## Jobs

| Operation          | z/OSMF | FTP | Backend         | Middleware | SDK | CLI | VSCE |
| ------------------ | ------ | --- | --------------- | ---------- | --- | --- | ---- |
| _Get job status_   | ✅     | ✅  | ✅              | ✅         | ✅  | ✅  | ❌   |
| _List jobs_        | ✅     | ✅  | 🚧 <sup>1</sup> | ✅         | ✅  | ✅  | ✅   |
| _List spool files_ | ✅     | ✅  | ✅              | ✅         | ✅  | ✅  | ✅   |
| _Read spool file_  | ✅     | ✅  | ✅              | ✅         | ✅  | ✅  | ✅   |
| _Get job JCL_      | ✅     | ❌  | ✅              | ✅         | ✅  | ✅  | ✅   |
| _Submit job_       | ✅     | ✅  | ✅              | ❌         | ❌  | ❌  | ❌   |
| Hold job           | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Release job        | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Change job class   | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ➖   |
| Cancel job         | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| _Delete job_       | ✅     | ✅  | ✅              | ❌         | ❌  | ❌  | ❌   |

1. Listing by prefix and status is not supported

## Console

| Operation                | z/OSMF | FTP | Backend         | Middleware | SDK | CLI | VSCE |
| ------------------------ | ------ | --- | --------------- | ---------- | --- | --- | ---- |
| Issue MVS command        | ✅     | ❌  | ✅ <sup>1</sup> | ✅         | ✅  | ✅  | ❌   |
| Get MVS command response | ✅     | ❌  | ❌              | ❌         | ❌  | ❌  | ❌   |
| Get MVS detect result    | ✅     | ❌  | ❌              | ❌         | ❌  | ➖  | ➖   |

1. Requires APF authorization

## TSO

| Operation                | z/OSMF | FTP | Backend | Middleware | SDK | CLI | VSCE |
| ------------------------ | ------ | --- | ------- | ---------- | --- | --- | ---- |
| Start TSO address space  | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Start TSO app            | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ➖   |
| Issue TSO command        | ✅     | ❌  | ❌      | ❌         | ✅  | ❌  | ❌   |
| Get TSO command response | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Send TSO message         | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Send TSO app message     | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ➖   |
| Ping TSO address space   | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Receive TSO message      | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Receive TSO app message  | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ➖   |
| Stop TSO address space   | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |

## Other

| Operation            | z/OSMF | FTP | Backend | Middleware | SDK | CLI | VSCE |
| -------------------- | ------ | --- | ------- | ---------- | --- | --- | ---- |
| Read system log      | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ➖   |
| Get server status    | ✅     | ❌  | ❌      | ❌         | ❌  | ❌  | ❌   |
| Change user password | ✅     | ❌  | ❌      | ❌         | ❌  | ➖  | ➖   |
| Issue SSH command    | ❌     | ❌  | ❌      | ❌         | ✅  | ❌  | ❌   |
