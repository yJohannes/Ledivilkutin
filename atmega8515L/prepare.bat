@echo off
setlocal

set "ROOT=%~dp0..\"
set "OUT=%ROOT%atmega8515L\build"

if exist "%OUT%" rmdir /s /q "%OUT%"

mkdir "%OUT%"

REM Copy main.c
copy "%ROOT%atmega8515L\main.c" "%OUT%\"

REM Copy simulation and mpu6050 + i2c files
copy "%ROOT%c\mpu6050_i2c\src\i2c_avr.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c_config.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\i2c.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050_regs.h" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050.c" "%OUT%\"
copy "%ROOT%c\mpu6050_i2c\src\mpu6050.h" "%OUT%\"

copy "%ROOT%c\sim\include\sim\config.h" "%OUT%\"
copy "%ROOT%c\sim\include\sim\simulation.h" "%OUT%\"
copy "%ROOT%c\sim\include\sim\vec2.h" "%OUT%\"

copy "%ROOT%c\sim\src\config.c" "%OUT%\"
copy "%ROOT%c\sim\src\int_arithmetic.c" "%OUT%\"
copy "%ROOT%c\sim\src\int_arithmetic.h" "%OUT%\"
copy "%ROOT%c\sim\src\simulation.c" "%OUT%\"
copy "%ROOT%c\sim\src\utils.h" "%OUT%\"

echo Firmware sources prepared in:
echo %OUT%
