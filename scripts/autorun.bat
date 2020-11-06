@echo off

SET svrid=Autorun_jc
SET script="%~dp0%2"

if "%1"=="install" (
    if "%2" == "" (
        echo please input script file!
    ) else (
        if not exist %script% (
            echo script file not exists: %script%
        ) else (
nssm install %svrid% %script% && ^
nssm set %svrid% AppDirectory %~dp0 && ^
nssm set %svrid% DisplayName "Autorun.Jc Service" && ^
nssm set %svrid% Start SERVICE_AUTO_START
        )
    )
) else if "%1"=="remove" (
    nssm stop %svrid%
    nssm remove %svrid% confirm
) else if "%1"=="restart" (
    nssm restart %svrid%
) else if "%1"=="start" (
    nssm start %svrid%
) else if "%1"=="stop" (
    nssm stop %svrid%
) else if "%1"=="status" (
    nssm status %svrid%
) else (
    echo usage: autostart.bat ^<command^> [^<script^>]
    echo.
    echo *** Administrator access is needed! ***
    echo.
    echo These are common commands used in various situations:
    echo    install    Install service
    echo    remove     Uninstall service
    echo    restart    Restart service
    echo    start      Start service
    echo    stop       Stop service
    echo    status     Check service status
)
