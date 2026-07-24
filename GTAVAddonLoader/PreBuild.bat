@echo off
setlocal enabledelayedexpansion

set "ProjDir=%~dp0"
copy /b "%ProjDir%main.cpp" +,,
copy /b/v/y "%ProjDir%GitInfo.h.template" "%ProjDir%GitInfo.h"

:: Get short commit hash
FOR /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --short HEAD`) DO (
  SET GitInfo=%%F
)

:: Detect if working tree is dirty
SET GitDirty=
git diff --quiet
if errorlevel 1 (
  SET GitDirty=-dirty
)

IF NOT "%CI%"=="" (
  SET GitDirty=%GitDirty%-auto
)

IF "%GitDirty%"=="" (
  :: Get commit date in UTC
  FOR /F "tokens=* USEBACKQ" %%F IN (`git log -1 --format^=%%cd --date^=format-local:"%%Y-%%m-%%dT%%H:%%M:%%SZ"`) DO (
    SET GitDate=%%F
  )
) ELSE (
  SET GitDate=%date%T%time%L
)

:: Output to header
ECHO #define GIT_HASH "%GitInfo%">>"%ProjDir%GitInfo.h"
ECHO #define GIT_DIFF "%GitDirty%">>"%ProjDir%GitInfo.h"
ECHO #define GIT_DATE "%GitDate%">>"%ProjDir%GitInfo.h"
