@echo off
setlocal

set "ROOT=%~dp0..\"
set "BUILD=%ROOT%build\"
set "OUT=%BUILD%atmega8515L\"

cd /d "%OUT%"

echo Current working directory: "%CD%"

REM Check that firmware.elf exists
if not exist firmware.elf (
    echo Error: firmware.elf not found. Run the build script first.
    exit /b 1
)

REM Generate assembly listing with interleaved C source
avr-objdump -d -S firmware.elf > firmware.lst

if errorlevel 1 exit /b 1

echo Assembly listing generated: firmware.lst

REM Open the listing in Notepad
notepad firmware.lst

endlocal
