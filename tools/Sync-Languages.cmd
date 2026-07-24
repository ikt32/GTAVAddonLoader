@echo off
powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%~dp0Sync-Languages.ps1"
if errorlevel 1 exit /b 1
if "%~1"=="" exit /b 0
call "%~1Prebuild.bat" "%~1"
exit /b %errorlevel%
