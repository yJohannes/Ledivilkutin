import math

from config import LEDS_X, LEDS_Y, SIM_PARTICLE_COUNT
from vec2 import Vec2


def create_particles() -> tuple[list[Vec2], list[Vec2]]:
    p = [Vec2(0, 0) for _ in range(SIM_PARTICLE_COUNT)]
    pp = [Vec2(0, 0) for _ in range(SIM_PARTICLE_COUNT)]

    cx = 128
    cy = 128

    # Spiral spacing
    spacing = 15
    angle_step = 0.3

    for i in range(SIM_PARTICLE_COUNT):
        angle = i * angle_step
        radius = spacing * angle / (2 * math.pi)

        px = int(cx + radius * math.cos(angle))
        py = int(cy + radius * math.sin(angle))

        p[i] = Vec2(px, py)
        pp[i] = Vec2(px, py)

    return p, pp

# from config import LEDS_X, LEDS_Y, SIM_PARTICLE_COUNT
# from vec2 import Vec2


# def create_particles() -> tuple[list[Vec2], list[Vec2]]:
#     p = [Vec2(0, 0) for _ in range(SIM_PARTICLE_COUNT)]
#     pp = [Vec2(0, 0) for _ in range(SIM_PARTICLE_COUNT)]

#     for y in range(LEDS_Y):
#         for x in range(LEDS_X):
#             i = LEDS_X * y + x

#             if i >= SIM_PARTICLE_COUNT:
#                 break

#             px = x * LEDS_X // 2 + LEDS_X
#             py = y * LEDS_X // 2 + LEDS_X

#             p[i] = Vec2(px, py)
#             pp[i] = Vec2(px, py)

#     return p, pp
