@echo off

SET script=%~dp0%2

if "%1"=="install" (
    if "%2" == "" (
        echo please input script file!
    ) else (
        if not exist "%script%" (
            echo script file not exists: %script%
        ) else (
nssm install Autorun_jc "%script%" && ^
nssm set Autorun_jc AppDirectory "%~dp0" && ^
nssm set Autorun_jc DisplayName "Autorun.Jc Service" && ^
nssm set Autorun_jc Start SERVICE_AUTO_START
        )
    )
) else if "%1"=="remove" (
    nssm remove Autorun_jc confirm
) else if "%1"=="restart" (
    nssm restart Autorun_jc
) else if "%1"=="start" (
    nssm start Autorun_jc
) else if "%1"=="stop" (
    nssm stop Autorun_jc
) else if "%1"=="status" (
    nssm status Autorun_jc
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
