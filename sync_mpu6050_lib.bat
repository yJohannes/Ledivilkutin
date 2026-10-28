@echo off
setlocal

REM Define project and destination paths
set "ROOT=%~dp0"
set "SOURCE=%ROOT%libraries\MPU6050_I2C"
set "SRC_CODE=%ROOT%c\mpu6050_i2c\src"
set "DEST=%USERPROFILE%\Documents\Arduino\libraries\MPU6050_I2C"

REM Check that the project source directory exists
if not exist "%SRC_CODE%\." (
    echo ERROR: Source code not found: "%SRC_CODE%"
    exit /b 1
)

REM Check that the Arduino library exists
if not exist "%SOURCE%\library.properties" (
    echo ERROR: Library not found: "%SOURCE%"
    exit /b 1
)

REM Create the library src directory if necessary
if not exist "%SOURCE%\src" (
    mkdir "%SOURCE%\src"
)

REM Copy project source files into the library src directory
robocopy "%SRC_CODE%" "%SOURCE%\src" /E /XD .git

REM Robocopy exit codes below 8 indicate success
if errorlevel 8 (
    echo ERROR: Source copy failed.
    exit /b 1
)

REM Create the Arduino libraries directory if necessary
if not exist "%USERPROFILE%\Documents\Arduino\libraries" (
    mkdir "%USERPROFILE%\Documents\Arduino\libraries"
)

REM Synchronize the complete library with the Arduino installation
REM /MIR mirrors files, including deletions
REM /XD .git excludes Git directories
robocopy "%SOURCE%" "%DEST%" /MIR /XD .git

REM Check whether library synchronization failed
if errorlevel 8 (
    echo ERROR: Library synchronization failed.
    exit /b 1
)

REM Report successful synchronization
echo Library synchronized successfully.
exit /b 0


@REM  @echo off
@REM  setlocal

@REM  set "SOURCE=%~dp0libraries\MPU6050_I2C"
@REM  set "DEST=%USERPROFILE%\Documents\Arduino\libraries\MPU6050_I2C"

@REM  if not exist "%SOURCE%\library.properties" (
@REM      echo ERROR: Library not found: "%SOURCE%"
@REM      exit /b 1
@REM  )

@REM  if not exist "%USERPROFILE%\Documents\Arduino\libraries" (
@REM      mkdir "%USERPROFILE%\Documents\Arduino\libraries"
@REM  )

@REM  robocopy "%SOURCE%" "%DEST%" /MIR /XD .git

@REM  if errorlevel 8 (
@REM      echo ERROR: Library synchronization failed.
@REM      exit /b 1
@REM  )

@REM  echo Library synchronized successfully.
@REM  exit /b 0
