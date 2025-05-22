| Method    | Size             | Upload Time (s) | Download Time (s) |
| --------- | ---------------- | --------------- | ----------------- |
| SFTP      | 100MB            | 2.3             | 3.5               |
| SSH (B64) | 100MB            | 4.3             | 4.8               |
| SSH (B85) | 100MB            | 6.9             | 4.9               |
| SSH (Raw) | 100MB            | TBD             | TBD <sup>1</sup>  |
| z/OSMF    | 100MB            | 2.6             | 5.0               |
| ZNP SDK   | N/A <sup>2</sup> |                 |                   |

1. Download in SSH raw mode is hurt by fixed chunk size of 2048
2. Lacks streaming support, hangs for files 100KB or larger
