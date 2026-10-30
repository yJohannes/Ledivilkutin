
@echo off
if "%~1"=="gui" (
    build\c\gui\fluid_simulation_gui.exe
) else if "%~1"=="a" (
    echo a
) else (
    echo Usage: run ^<empty^|gui^>
    exit /b 1
)
