# Ledivilkutin

## Logs

2/11/2026
- Struggled a lot with efficient LED PWM. Still finished it with hopefully enough optimizations.
  Lots of assumptions, some quite aggressive, were made and I hope the performance doesn't sway.
- Finished the microcontroller code.

1/11/2026
- Made good progress with the microcontroller code.
- Wrote the main loop for the most part
- Added button interrupt

31/10/2026
- Prepare code for programming the microcontroller.
- Performance testing and optimizations.

30/10/2026
- Prepare code for programming the microcontroller.
- Performance testing and optimizations.

29/10/2026
- Finished porting the simulation.
- Achieved excellent stability and efficiency by using `__builtin_add_overflow` to not perform addition if overflow occurs.

28/10/2026
- Began working on a GUI using Dear ImGui to tweak parameters during runtime.

27/10/2026
- Simulation code almost fully ported.

26/10/2026
- Began porting the simulation to C.

25/10/2026
- Awesome new ideas and improvements to the fluid simulation.
- Big: update position every frame but apply acceleration every 2nd. By far the most stable the simulation has been.
- Calculate center of mass every 3rd frame and pull particles towards it. Works surprisingly well.
- Removed spatial grid to free up 139 B. The naive approach paired with pair only comparison (`i: [0, pcount), j: [i+1, pcount]`) should be fine here.
- Put Z-accelerometer also in standby as it's not used.

24/10/2026
- Finished writing I2C implementation for AVR and wrote MPU6050 headers.
- Organized project files.

23/10/2026
- Got the I2C driver working and reading acceleration values from the MPU6050 GY-521. Massive.

22/10/2026
- Made simulation use verlet integration for stability. Big upgrade.
- Began working on the I2C driver again.

20/10/2026
- Worked on particle separation and simulation stability.

19/10/2026
- Started working on a particle based fluid simulation for more fluid-like behaviour.
- Wrote a spatial grid for efficient queries and force calculations for neighbouring particles.

15/10/2026
- Soldered 127 LEDs, programming pins, button and battery leads.

14/10/2026
- Soldered 60 LEDs

1/10/2026 - 13/10/2026
- Worked on a grid based fluid simulation using CA Margolus algorithm.

30/9/2026
- Soldered MCU, LEDs, resistors and capacitor.
- Got the MPU6050 GY-521 for reading acceleration.

29/9/2026
- Bought 220 LEDs.

pre
- Started working on a grid based fluid simulation in Python.


## Programming the microcontroller checklist

### Before programming
- Set correct platform in `i2c_config.h`.
- Compile with -O2 not -Os.

### After programming
- Wire VCC, GND, AD0->GND, SDA->PB5 and SCL->PB6.
- Ask for an additional piece of two-sided tape for the accelerometer.

### Reprogramming
- Disconnect MPU6050.
