cd %~dp0
.\brook.exe client -s 81.69.241.25:6002 -p 431387@proxy --socks5 127.0.0.1:8172 2>&1 1>%~dp0brook.log
