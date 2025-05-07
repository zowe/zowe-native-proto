| Method    | Results                                                      |
| --------- | ------------------------------------------------------------ |
| SFTP      | 1MB: Upload/download each around 2-4 sec                     |
| SSH (B64) | 1MB: Upload in 0.5-0.7 sec, Download in 4-6 sec <sup>1</sup> |
| SSH (Raw) | 1MB: Upload in 0.7-1 sec, Download in ~10 sec <sup>1</sup>   |
| ZNP SDK   | ⚠️ No streaming, hangs if file is 100 KB or larger           |
| z/OSMF    | 1MB: Upload/download each around 2 sec                       |

1. Not sure why download is so slow, but ssh stdout chunk size seems fixed<br>(32768 in B64 mode, 2048 in raw mode)
