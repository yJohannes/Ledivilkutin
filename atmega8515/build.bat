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
copy "%ROOT%atmega8515\src\main.c" "%OUT%\"
copy "%ROOT%atmega8515\src\ledivilkutin.c" "%OUT%\"
copy "%ROOT%atmega8515\src\ledivilkutin.h" "%OUT%\"
copy "%ROOT%atmega8515\src\ledivilkutin_config.h" "%OUT%\"
copy "%ROOT%atmega8515\src\button.c" "%OUT%\"
copy "%ROOT%atmega8515\src\button.h" "%OUT%\"
copy "%ROOT%atmega8515\src\sim_timer.c" "%OUT%\"
copy "%ROOT%atmega8515\src\sim_timer.h" "%OUT%\"
copy "%ROOT%atmega8515\src\sensor.c" "%OUT%\"
copy "%ROOT%atmega8515\src\sensor.h" "%OUT%\"
copy "%ROOT%atmega8515\src\leds.c" "%OUT%\"
copy "%ROOT%atmega8515\src\leds.h" "%OUT%\"

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


    @REM  -Wconversion ^
    @REM  -Wsign-conversion ^
avr-gcc -mmcu=atmega8515 ^
    -O2 ^
    -ffunction-sections ^
    -fdata-sections ^
    -Wl,--gc-sections ^
    -Wall ^
    -Wextra ^
    -Wpedantic ^
    -Wshadow ^
    -Wundef ^
    -Wdouble-promotion ^
    -Wformat=2 ^
    -Wstrict-prototypes ^
    -Wmissing-prototypes ^
    -Werror=implicit-function-declaration ^
    -DF_CPU=8000000UL ^
    -DI2C_PLATFORM_AVR ^
    -DSIM_INTEGRATION_ENABLED ^
    -DSIM_COLLISIONS_ENABLED ^
    -DSIM_COM_ENABLED ^
    -DSIM_FRICTION_ENABLED ^
    -DSIM_USE_POINTER_ITERATION ^
    main.c ^
    ledivilkutin.c ^
    button.c ^
    sim_timer.c ^
    leds.c ^
    i2c.c ^
    mpu6050.c ^
    sensor.c ^
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
