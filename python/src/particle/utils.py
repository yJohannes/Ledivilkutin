from int_limits import (
    INT4_MIN,
    INT4_MAX,
    INT16_MIN,
    INT16_MAX,
    INT8_MIN,
    INT8_MAX,
)
from config import (
    LEDS_X,
    LEDS_Y,
    WINDOW_H,
    WINDOW_W,
)

def min_(a, b):
    return a if a < b else b


def max_(a, b):
    return a if a > b else b


def sign(x):
    return (x > 0) - (x < 0)


def clamp2(a, b, limit):
    total = a + b

    if total <= limit:
        return a, b

    a_new = a * limit // total
    b_new = limit - a_new

    return a_new, b_new


# x = int8
def clamp_int4(x):
    if x < INT4_MIN:
        return INT4_MIN
    if x > INT4_MAX:
        return INT4_MAX
    return x


def scale_int16_to_int7(value: int) -> int:
    value = max(-32768, min(32767, value))
    return round(value * 63 / 32767)


def scale_int16_to_int7_symmetric(value: int) -> int:
    value = max(-32768, min(32767, value))
    return round(value * 64 / 32768)


def scale_int16_to_int4(value: int) -> int:
    value = max(-32768, min(32767, value))
    return round(value * 7 / 32767)


def scale_int16_to_int8(value: int) -> int:
    value = max(-32768, min(32767, value))
    return round(value * 127 / 32767)


def sim_to_screen_coordinate(sim_x, sim_y):
    return (int(sim_x / 256 * WINDOW_W), int(sim_y / 256 * WINDOW_H))
