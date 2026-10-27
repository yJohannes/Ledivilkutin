
@echo off
setlocal

set "SOURCE=%~dp0libraries\MPU6050_I2C"
set "DEST=%USERPROFILE%\Documents\Arduino\libraries\MPU6050_I2C"

if not exist "%SOURCE%\library.properties" (
    echo ERROR: Library not found: "%SOURCE%"
    exit /b 1
)

if not exist "%USERPROFILE%\Documents\Arduino\libraries" (
    mkdir "%USERPROFILE%\Documents\Arduino\libraries"
)

robocopy "%SOURCE%" "%DEST%" /MIR /XD .git

if errorlevel 8 (
    echo ERROR: Library synchronization failed.
    exit /b 1
)

echo Library synchronized successfully.
exit /b 0
