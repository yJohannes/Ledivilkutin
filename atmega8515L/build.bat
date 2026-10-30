@echo off
setlocal


REM Change cwd to atmega8515L/build
cd /d "%~dp0build"
echo Current working directory: "%CD%"


avr-gcc -mmcu=atmega8515 -Os -DF_CPU=8000000UL ^
    -DSIM_INTEGRATION_ENABLED ^
    -DSIM_COLLISIONS_ENABLED ^
    -DSIM_COM_ENABLED ^
    -DSIM_FRICTION_ENABLED ^
    -DSIM_USE_POINTER_ITERATION ^
    main.c ^
    i2c.c ^
    mpu6050.c ^
    config.c ^
    int_arithmetic.c ^
    simulation.c ^
    -o firmware.elf

if errorlevel 1 exit /b 1

avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
if errorlevel 1 exit /b 1

echo Build successful: firmware.hex
