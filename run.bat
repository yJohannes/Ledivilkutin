
@echo off
if "%~1"=="gui" (
    build\sim\c\gui\fluid_simulation_gui.exe
) else if "%~1"=="a" (
    echo a
) else (
    echo Usage: run ^<cli^|gui^|uci^>
    exit /b 1
)
