
@echo off
if "%~1"=="gui" (
    build\c\gui\fluid_simulation_gui.exe
) else (
    echo Usage: run ^<_^|gui^>
    exit /b 1
)
