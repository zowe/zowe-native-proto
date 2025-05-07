| Method    | Size  | Upload Time (s)  | Download Time (s) |
| --------- | ----- | ---------------- | ----------------- |
| SFTP      | 1MB   | 2-4              | 2-4               |
| SSH (B64) | 1MB   | 2-4              | 2-4               |
| SSH (Raw) | 1MB   | 2-3              | ~8 <sup>1</sup>   |
| z/OSMF    | 1MB   | ~2               | ~2                |
| SFTP      | 10MB  | 15-30            | 15-30             |
| SSH (B64) | 10MB  | ~30              | ~22               |
| SSH (B85) | 10MB  | ~16              | ~11               |
| SSH (Raw) | 10MB  | ~23              | ~72 <sup>1</sup>  |
| z/OSMF    | 10MB  | 10-25            | 10-25             |
| ZNP SDK   | 100KB | N/A <sup>2</sup> |                   |

1. Download in SSH raw mode is hurt by fixed chunk size of 2048
2. Lacks streaming support, hangs for files 100KB or larger
