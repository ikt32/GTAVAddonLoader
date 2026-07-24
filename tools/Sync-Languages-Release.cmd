@echo off
call "%~dp0Sync-Languages.cmd"
if errorlevel 1 exit /b 1
call "%~dp0..\GTAVAddonLoader\Prebuild.bat"
exit /b %errorlevel%
