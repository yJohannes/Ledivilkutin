@echo off
setlocal

set "ROOT=%~dp0..\"
set "BUILD=%ROOT%build\"
set "OUT=%BUILD%atmega8515\"
set "PROJECT_OUT=%ROOT%atmega8515\Ledivilkutin\Ledivilkutin\"


mkdir "%BUILD%"
if exist "%OUT%" rmdir /s /q "%OUT%"
mkdir "%OUT%"

REM Copy main.c
copy "%ROOT%atmega8515\main.c" "%OUT%\"

REM Copy simulation and mpu6050 + i2c files
copy "%ROOT%c\mpu6050_i2c\src\i2c_avr.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c_config.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050_regs.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050.h" "%OUT%\"

copy "%ROOT%c\sim\include\sim\sim_config.h" "%OUT%\"
copy "%ROOT%c\sim\include\sim\sim.h" "%OUT%\"
copy "%ROOT%c\sim\include\sim\vec2.h" "%OUT%\"

copy "%ROOT%c\sim\src\sim_config.c" "%OUT%\"
copy "%ROOT%c\sim\src\sim.c" "%OUT%\"
copy "%ROOT%c\sim\src\int_arithmetic.c" "%OUT%\"
copy "%ROOT%c\sim\src\int_arithmetic.h" "%OUT%\"
copy "%ROOT%c\sim\src\utils.h" "%OUT%\"


REM Copy source and header files to the project directory
@REM  if not exist "%PROJECT_OUT%" mkdir "%PROJECT_OUT%"
@REM  copy "%OUT%\*.c" "%PROJECT_OUT%\"
@REM  copy "%OUT%\*.h" "%PROJECT_OUT%\"

echo.
echo Firmware sources prepared in:
echo %OUT%

cd /d "%OUT%"

@REM  -DSIM_USE_POINTER_ITERATION ^

avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -DF_CPU=8000000UL ^
    -DSIM_INTEGRATION_ENABLED ^
    -DSIM_COLLISIONS_ENABLED ^
    -DSIM_COM_ENABLED ^
    -DSIM_FRICTION_ENABLED ^
    main.c ^
    i2c.c ^
    mpu6050.c ^
    sim_config.c ^
    int_arithmetic.c ^
    sim.c ^
    -o firmware.elf

if errorlevel 1 exit /b 1

avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
if errorlevel 1 exit /b 1

echo.
echo Build successful: firmware.hex

if errorlevel 1 exit /b 1

call "%ROOT%\atmega8515\size.bat"

if errorlevel 1 exit /b 1


endlocal
