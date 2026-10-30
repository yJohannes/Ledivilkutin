@echo off
pushd "%~dp0build" || exit /b 1

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
