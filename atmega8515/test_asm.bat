@echo off
setlocal

set "ROOT=%~dp0..\"
set "BUILD=%ROOT%build\"
set "OUT=%BUILD%atmega8515\tests\"

if not exist "%BUILD%" mkdir "%BUILD%"
if exist "%OUT%" rmdir /s /q "%OUT%"
mkdir "%OUT%"

copy "%~dp0tests\asm.c" "%OUT%"

cd /d "%OUT%"

REM Generate assembly
avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -std=c11 ^
    -S asm.c ^
    -o asm.s

if errorlevel 1 exit /b 1

REM Build ELF
avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -std=c11 ^
    asm.c ^
    -o asm.elf

if errorlevel 1 exit /b 1

REM Print size information
echo.
echo ================= SIZE =================
avr-size -C --mcu=atmega8515 asm.elf
if errorlevel 1 exit /b 1

echo.
echo ================= ASSEMBLY ==============
echo Opening asm.s in Notepad...
start "" notepad.exe "%OUT%asm.s"

endlocal
