| Method    | Size | Results                                              |
| --------- | ---- | ---------------------------------------------------- |
| SFTP      | 1MB  | Upload/download each around 2-4 sec                  |
| SSH (B64) | 1MB  | Upload/download each around 2-4 sec                  |
| SSH (Raw) | 1MB  | Upload in 2-3 sec, Download in ~8 sec <sup>1</sup>   |
| z/OSMF    | 1MB  | Upload/download each around 2 sec                    |
| SFTP      | 10MB | Upload/download each around 15-30 sec                |
| SSH (B64) | 10MB | Upload in ~30 sec, Download in ~22 sec               |
| SSH (Raw) | 10MB | Upload in ~23 sec, Download in ~1.2 min <sup>1</sup> |
| z/OSMF    | 10MB | Upload/download each around 10-25 sec                |
| ZNP SDK   | N/A  | ⚠️ No streaming, hangs with 100 KB or larger         |

1. Download in SSH raw mode is hurt by fixed chunk size of 2048
