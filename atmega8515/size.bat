@echo off
setlocal

set "ROOT=%~dp0..\"
set "BUILD=%ROOT%build\"
set "OUT=%BUILD%atmega8515\"

cd /d "%OUT%"


echo.
echo === Column meanings ===
echo text = Program code and read-only data in Flash
echo data = Initialized variables (uses Flash and SRAM)
echo bss  = Uninitialized/zero-initialized variables in SRAM
echo dec  = Sum of text + data + bss
echo hex  = Total in hexadecimal
echo.

avr-size firmware.elf

echo.
echo Flash usage = text + data
echo SRAM usage  = data + bss
echo.

popd

endlocal
