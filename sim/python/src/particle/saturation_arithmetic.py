from int_limits import INT8_MAX, INT8_MIN, UINT8_MIN, UINT8_MAX


def add_u8_s8(x, y):
    result = x + y
    overflow = result < UINT8_MIN or result > UINT8_MAX
    return max(UINT8_MIN, min(UINT8_MAX, result)), overflow


def add_s8(x, y):
    result = x + y
    overflow = result < INT8_MIN or result > INT8_MAX
    return max(INT8_MIN, min(INT8_MAX, result)), overflow


def add_of_u8(x, y):
    result = x + y
    overflow = result < 0 or result > 255
    return max(0, min(255, result)), overflow


def sub_of_u8(x, y):
    result = x - y
    overflow = result < 0 or result > 255
    return max(0, min(255, result)), overflow


def add_u8(x, y):
    return max(0, min(255, x + y))


def sub_u8(x, y):
    return max(0, min(255, x - y))


def clamp_u8(x: int) -> int:
    return max(0, min(255, x))
