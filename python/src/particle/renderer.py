import pygame

from config import (
    LED_CELL_SIZE,
    LEDS_X,
    LEDS_Y,
    RENDERER_DRAW_PARTICLES,
    SIM_COM_EFFECTIVE_RADIUS,
    SIM_PARTICLE_COUNT,
    WINDOW_H,
    WINDOW_W,
)
from utils import sim_to_screen_coordinate


def led_color(intensity):
    r = int(intensity * 255)
    g = int(intensity * 141)
    b = int(intensity * 54)

    return r, g, b


def render(screen, font, sim):
    screen.fill((10, 46, 2))

    intensity_map = [0 for _ in range(LEDS_X * LEDS_Y)]

    for i in range(SIM_PARTICLE_COUNT):
        led_x = sim.p[i].x >> 4
        led_y = sim.p[i].y >> 4

        sq_idx = LEDS_X * led_y + led_x

        intensity_map[sq_idx] = 3

        # Right
        if led_x + 1 < LEDS_X:
            intensity_map[sq_idx + 1] = min(intensity_map[sq_idx + 1] + 1, 3)

        # Left
        if led_x > 0:
            intensity_map[sq_idx - 1] = min(intensity_map[sq_idx - 1] + 1, 3)

        # Down
        if led_y + 1 < LEDS_Y:
            intensity_map[sq_idx + LEDS_X] = min(intensity_map[sq_idx + LEDS_X] + 1, 3)

        # Up
        if led_y > 0:
            intensity_map[sq_idx - LEDS_X] = min(intensity_map[sq_idx - LEDS_X] + 1, 3)

    # # Draw LED grid
    for y in range(LEDS_Y):
        for x in range(LEDS_X):
            rect = pygame.Rect(
                x * LED_CELL_SIZE,
                y * LED_CELL_SIZE,
                LED_CELL_SIZE,
                LED_CELL_SIZE,
            )

            # color = led_color(0.33 * intensity_map[y * LEDS_X + x])
            INTENSITY = (0.0, 0.15, 0.4, 0.9)

            color = led_color(INTENSITY[intensity_map[y * LEDS_X + x]])

            pygame.draw.rect(screen, color, rect)
            pygame.draw.rect(screen, (77, 43, 17), rect, 1)

    # Draw particles
    if RENDERER_DRAW_PARTICLES:
        for i in range(SIM_PARTICLE_COUNT):
            x = sim.p[i].x * (LEDS_X * LED_CELL_SIZE) // 256
            y = sim.p[i].y * (LEDS_Y * LED_CELL_SIZE) // 256

            pygame.draw.circle(
                screen,
                led_color(0.75),
                (x, y),
                LED_CELL_SIZE / 6,
            )

    screen_com_x, screen_com_y = sim_to_screen_coordinate(
        sim.com_u16.x // SIM_PARTICLE_COUNT, sim.com_u16.y // SIM_PARTICLE_COUNT
    )

    # Draw center of mass & effective radius
    pygame.draw.circle(
        screen, (255, 255, 255), (screen_com_x, screen_com_y), LED_CELL_SIZE / 8
    )

    pygame.draw.circle(
        screen,
        (128, 128, 128),
        (screen_com_x, screen_com_y),
        SIM_COM_EFFECTIVE_RADIUS,
        width=2,
    )

    font = pygame.font.Font(None, 22)

    text = font.render(
        f"ax={sim.acceleration.x},ay={sim.acceleration.y}", True, (255, 255, 255)
    )

    screen.blit(text, (0, 0))

    mouse_x, mouse_y = pygame.mouse.get_pos()

    center_x = WINDOW_W / 2
    center_y = WINDOW_H / 2

    # --------------------------------------------------------
    # Display acceleration direction
    # --------------------------------------------------------

    ax = sim.acceleration.x
    ay = sim.acceleration.y

    arrow_scale = 2

    end_x = center_x + ax * arrow_scale
    end_y = center_y + ay * arrow_scale

    pygame.draw.line(screen, (230, 230, 230), (center_x, center_y), (end_x, end_y), 3)
    pygame.draw.circle(screen, (230, 230, 230), (center_x, center_y), 4)
