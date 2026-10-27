# README
Hi

## Logs

26/10/2026
- Began rewriting the simulation in C.

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
- Bought 220 LEDs

pre
- Started working on a grid based fluid simulation.
