@echo off
setlocal

set "ROOT=%~dp0..\"
set "BUILD=%ROOT%build\"
set "OUT=%BUILD%atmega8515\tests\"

if not exist "%BUILD%" mkdir "%BUILD%"
if exist "%OUT%" rmdir /s /q "%OUT%"
mkdir "%OUT%"

copy "%~dp0tests\test.c" "%OUT%"

cd /d "%OUT%"

REM Generate assembly
avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -std=c11 ^
    -S test.c ^
    -o test.s

if errorlevel 1 exit /b 1

REM Build ELF
avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -std=c11 ^
    test.c ^
    -o test.elf

if errorlevel 1 exit /b 1

REM Print size information
echo.
echo ================= SIZE =================
avr-size -C --mcu=atmega8515 test.elf
if errorlevel 1 exit /b 1

echo.
echo ================= ASSEMBLY ==============
echo Opening test.s in Notepad...
start "" notepad.exe "%OUT%test.s"

endlocal
