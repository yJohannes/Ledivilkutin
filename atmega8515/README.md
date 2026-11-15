# Requirements
- avr-gcc

# Building firmware
```
cd atmega8515
build
```

# Uploading firmware to ATmega8515

```
cd build\atmega8515
```

## Check device connection

### COM3
```
avrdude -p m8515 -c stk500v1 -P COM3 -b 19200 -v
```

### COM4
```
avrdude -p m8515 -c stk500v1 -P COM4 -b 19200 -v
```

## Upload firmware

### COM3
```
avrdude -p m8515 -c stk500v1 -P COM3 -b 19200 -U flash:w:firmware.hex:i
```

### COM4
```
avrdude -p m8515 -c stk500v1 -P COM4 -b 19200 -U flash:w:firmware.hex:i
```
