# Uploading firmware to ATmega8515

## Check device connection
```
avrdude -p m8515 -c stk500v1 -P COM3 -b 19200 -v
```

## Upload firmware

```
avrdude -p m8515 -c stk500v1 -P COM3 -b 19200 -U flash:w:firmware.hex:i
```
