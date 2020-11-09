cd %~dp0
start .\brook.exe server -l :5998 -p 431387@proxy 2>&1 1>%~dp0brook.log
.\frpc.exe -c .\frpc.ini 2>&1 1>%~dp0frpc.log
