
# 1/11/2026

## Memory usage on ATmega16

| Optimization | Text (bytes) | Data (bytes) | BSS (bytes) | Total (bytes) | Hex   |
| ------------ | ------------ | ------------ | ----------- | ------------- | ----- |
| `-Og`        | 2,090        | 0            | 309         | 2,399         | `95f` |
| `-O2`        | 2,420        | 0            | 309         | 2,729         | `aa9` |
| `-O3`        | 3,060        | 0            | 309         | 3,369         | `d29` |

## Cycles per `simulation_step` on ATmega16

| Optimization | Value 1 | Value 2 | Cycles  |
| ------------ | ------- | ------- | ------- |
| `-Og`        | 400,438 | 726,696 | 326,258 |
| `-Os`        | 400,434 | 659,806 | 259,372 |
| `-O1`        | 400,434 | 702,068 | 301,634 |
| `-O2`        | 400,432 | 643,952 | 243,520 |
| `-O3`        | 400,432 | 644,177 | 243,745 |


# 2/11/2026
## Simulations on ATmega16

Clock cycle budget for 19 FPS @ 4 MHz:
$$B=\frac{8\,000\,000}{19}=421\,052.63$$
Equivalent to 52.632 ms per frame

### Results running everything excluding MPU6050 and I2C:

| Frame | Cycles  | Budget usage |
| ----- | ------- | ------------ |
| 1     | 18,242  | 4.3%         |
| 2     | 660,538 | 156.9%       |
| 3     | 354,684 | 84.2%        |
| 4     | 402,239 | 95.5%        |
| 5     | 407,326 | 96.7%        |
| 6     | 412,734 | 98.0%        |
| 7     | 417,593 | 99.2%        |
| 8     | 419,361 | 99.6%        |
| 9     | 417,676 | 99.2%        |
| 10    | 423,343 | 100.5%       |
| 11    | 418,032 | 99.3%        |
| 12    | 420,951 | 100.0%       |
| 13    | 423,851 | 100.7%       |
| 14    | 423,165 | 100.5%       |
| 15    | 415,479 | 98.7%        |
| 16    | 423,880 | 100.7%       |
| 17    | 417,631 | 99.2%        |
| 18    | 420,227 | 99.8%        |
| 19    | 423,985 | 100.7%       |


Ignoring the first two measurements:

| Metric  | Result         |
| ------- | -------------- |
| Average | 414,245 cycles |
| Median  | 418,032 cycles |
| Minimum | 354,684 cycles |
| Maximum | 423,985 cycles |
| Budget  | 421,053 cycles |

Slight fluctuations around the budget. Very promising.

<!--
18242
678780
1033464
1435703
1843029
2255763
2673356
3092717
3510393
3933736
4351768
4772719
5196570
5619735
6035214
6459094
6876725
7296952
7720937
-->

### Results with `simulation_step` commented out excluding MPU6050 and I2C

The differences between consecutive measurements are:

| Frame | Cycles  | Difference from budget |
| ----- | ------- | ---------------------- |
| 1     | 420,378 | -675                   |
| 2     | 421,083 | +30                    |
| 3     | 420,645 | -408                   |
| 4–10  | 420,864 | -189                   |
| 11    | 431,769 | +10,716                |
| 12    | 409,959 | -11,094                |
| 13–19 | 420,864 | -189                   |

These results indicate timers are working as intended.

<!--
18242
438620
859703
1280348
1701212
2122076
2542940
2963804
3384668
3805532
4237301
4647260
5068124
5488988
5909852
6330716
6751580
7172444
7593308
8014172
 -->

## Memory usage

| Optimization | Text (bytes) | Data (bytes) | BSS (bytes) | Total (bytes) | Hex   |
| ------------ | ------------ | ------------ | ----------- | ------------- | ----- |
| `-O2`        | 3,102        | 108          | 377         | 3,587         | `e03` |
| `-Os`        | 2,648        | 108          | 377         | 3,133         | `c3d` |
