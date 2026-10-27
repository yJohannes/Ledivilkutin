import math

from collections import deque

import serial.serialutil
import pygame
import serial

GRID_W = 16
GRID_H = 16

CELL_SIZE = 65

WINDOW_W = GRID_W * CELL_SIZE
WINDOW_H = GRID_H * CELL_SIZE

FPS = 15

NUM_SQUARES = GRID_W * GRID_H


d = [[0 for x in range(GRID_W)] for y in range(GRID_H)]
vx = [[0 for x in range(GRID_W)] for y in range(GRID_H)]
vy = [[0 for x in range(GRID_W)] for y in range(GRID_H)]


# 0             = air
# 1..9          = partially filled water cell
# 10            = normal water
# 11..15        = compressed water
PREFERRED_DENSITY_AIR = 0
REST_DENSITY = 10
MAX_DENSITY = 7

# density/timestep
MAX_VELOCITY = 7

# velocity/timestep
FRICTION = 1

# density/timestep
SPREAD_RATE = 1

INT4_MIN = -8
INT4_MAX = 7
UINT8_MIN = 0
UINT8_MAX = 256


ADD_AMOUNT = MAX_DENSITY

PORT = "COM4"

while True:
    print("Attemping to connect to " + PORT + "...")

    try:
        arduino = serial.Serial(PORT, 38400, timeout=1)
        break
    except serial.serialutil.SerialException as e:
        print(e)


# [y, x]
g_acceleration = [-2, 0]
g_draw_velocity = True


# Later optimize this by just using precomputed midpoint and inline this
# @param density 0-15
def prefers_air(density):
    midpoint = (PREFERRED_DENSITY_AIR + REST_DENSITY) / 2
    return density < midpoint


def min_(a, b):
    return a if a < b else b


def max_(a, b):
    return a if a > b else b


def is_valid_x(x):
    return 0 <= x and x < GRID_W


def is_valid_y(y):
    return 0 <= y and y < GRID_H


def is_valid(y, x):
    return is_valid_x(x) and is_valid_y(y)


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


# v = int8
def clamp_velocity(v):
    if v < -MAX_VELOCITY:
        return -MAX_VELOCITY
    if v > MAX_VELOCITY:
        return MAX_VELOCITY
    return v


def scale_int16_to_int4(value: int) -> int:
    value = max(-32768, min(32767, value))
    scaled = round((value + 32768) * 15 / 65535) - 8
    return max(-8, min(7, scaled))


def apply_friction(v):
    if v > 0:
        return max_(0, v - FRICTION)
    if v < 0:
        return min_(0, v + FRICTION)
    return 0


def process_2x2(y, x):

    # This is one 2x2 block of the shape
    # A B
    # C D
    A = d[y][x]
    B = d[y][x + 1]
    C = d[y + 1][x]
    D = d[y + 1][x + 1]

    vAx = vx[y][x]
    vAy = vy[y][x]

    vBx = vx[y][x + 1]
    vBy = vy[y][x + 1]

    vCx = vx[y + 1][x]
    vCy = vy[y + 1][x]

    vDx = vx[y + 1][x + 1]
    vDy = vy[y + 1][x + 1]

    # Desired transfers.
    # Only velocity pointing into this block is considered.

    ab = max_(0, vAx)  # A -> B
    ac = max_(0, vAy)  # A -> C

    ba = max_(0, -vBx)  # B -> A
    bd = max_(0, vBy)  # B -> D

    ca = max_(0, -vCy)  # C -> A
    cd = max_(0, vCx)  # C -> D

    db = max_(0, -vDy)  # D -> B
    dc = max_(0, -vDx)  # D -> C

    # 1. Source capacity
    # Each cell cannot give away more density than it contains.
    ab, ac = clamp2(ab, ac, A)
    ba, bd = clamp2(ba, bd, B)
    ca, cd = clamp2(ca, cd, C)
    db, dc = clamp2(db, dc, D)

    # 2. Destination capacity
    ab, db = clamp2(ab, db, MAX_DENSITY - B)
    ac, dc = clamp2(ac, dc, MAX_DENSITY - C)

    ba, ca = clamp2(ba, ca, MAX_DENSITY - A)
    bd, cd = clamp2(bd, cd, MAX_DENSITY - D)

    # Apply density transfers
    A += ba + ca - ab - ac
    B += ab + db - ba - bd
    C += ac + dc - ca - cd
    D += bd + cd - db - dc

    d[y][x] = A
    d[y][x + 1] = B
    d[y + 1][x] = C
    d[y + 1][x + 1] = D

    # If fluid leaving a direction is zero, then the cell
    # must be at rest and have no velocity.

    # No +x-movement
    # if vAx > 0 and ab == 0:
    #     vAx = 0

    # # No +y-movement
    # if vAy > 0 and ac == 0:
    #     vAy = 0

    # if vBx < 0 and ba == 0:
    #     vBx = 0

    # if vBy > 0 and bd == 0:
    #     vBy = 0

    # if vCx > 0 and cd == 0:
    #     vCx = 0

    # if vCy < 0 and ca == 0:
    #     vCy = 0

    # if vDx < 0 and dc == 0:
    #     vDx = 0

    # if vDy < 0 and db == 0:
    #     vDy = 0
    # ----------------------------------------------------
    # Transfer velocity
    #
    # Only a fraction of the source velocity follows
    # the transferred liquid.
    #
    # >> 2 = 1/4
    # ----------------------------------------------------

    # if ab:
    #     dv = ab >> 2
    #     vAx -= dv
    #     vBx += dv

    # if ac:
    #     dv = ac >> 2
    #     vAy -= dv
    #     vCy += dv

    # if ba:
    #     dv = ba >> 2
    #     vBx -= dv
    #     vAx += dv

    # if bd:
    #     dv = bd >> 2
    #     vBy -= dv
    #     vDy += dv

    # if ca:
    #     dv = ca >> 2
    #     vCy -= dv
    #     vAy += dv

    # if cd:
    #     dv = cd >> 2
    #     vCx -= dv
    #     vDx += dv

    # if db:
    #     dv = db >> 2
    #     vDy -= dv
    #     vBy += dv

    # if dc:
    #     dv = dc >> 2
    #     vDx -= dv
    #     vCx += dv

    # Update velocity
    # vx[y][x] = clamp_int4(vAx)
    # vy[y][x] = clamp_int4(vAy)

    # vx[y][x + 1] = clamp_int4(vBx)
    # vy[y][x + 1] = clamp_int4(vBy)

    # vx[y + 1][x] = clamp_int4(vCx)
    # vy[y + 1][x] = clamp_int4(vCy)

    # vx[y + 1][x + 1] = clamp_int4(vDx)
    # vy[y + 1][x + 1] = clamp_int4(vDy)


friction_counter = 0


# 1. apply acceleration to velocities
# 2. apply velocities to densities
# 3. apply friction
# 4. spread densities to neighbours
def simulate():
    # Apply acceleration
    ay, ax = g_acceleration

    for y in range(16):
        for x in range(16):

            # di = d[y][x]

            # Nothing to accelerate
            # Actually scrap this because it gives artificial momentum
            # if di == 0:
            #     vy[y][x] = 0
            #     vx[y][x] = 0
            #     continue

            # vy[y][x] + dvx/y can't overflow int8
            # vy[y][x] = clamp_velocity(vy[y][x] + ay)
            # vx[y][x] = clamp_velocity(vx[y][x] + ax)

            vy[y][x] = ay
            vx[y][x] = ax

    # Margolus CA algorithm

    # 1st pass
    #
    # Process 2x2 blocks
    for y in range(0, 16, 2):
        for x in range(0, 16, 2):
            process_2x2(y, x)

    # 2nd pass
    #
    # Shift cells by (1, 1) and do 2x2 blocks again.
    # Some blocks will become 1x1, 2x1 and 1x2, but it's fine.

    # Interior
    for y in range(1, 15, 2):
        for x in range(1, 15, 2):
            process_2x2(y, x)

    # Corners: do nothing

    # Top & bottom edge: 2x1 blocks
    for y in [0, 15]:
        for x in range(1, 15, 2):

            A = d[y][x]
            B = d[y][x + 1]

            vAx = vx[y][x]
            vAy = vy[y][x]

            vBx = vx[y][x + 1]
            vBy = vy[y][x + 1]

            ab = max_(0, vAx)  # A -> B
            ba = max_(0, -vBx)  # B -> A

            # 1. Source capacity
            # Each cell cannot give away more density than it contains.
            ab = min_(ab, A)
            ba = min_(ba, B)

            # 2. Destination capacity
            # Destination can hold at most MAX_DENSITY and
            # therefore receive at most MAX_DENSITY - <CELL DENSITY>
            ab = min_(ab, MAX_DENSITY - B)
            ba = min_(ba, MAX_DENSITY - A)

            # Apply transfers
            A += ba - ab
            B += ab - ba

            d[y][x] = A
            d[y][x + 1] = B

            if ab:
                dv = ab >> 2
                vAx -= dv
                vBx += dv

            if ba:
                dv = ba >> 2
                vBx -= dv
                vAx += dv

            # Update velocity
            vx[y][x] = clamp_int4(vAx)
            vy[y][x] = clamp_int4(vAy)

            vx[y][x + 1] = clamp_int4(vBx)
            vy[y][x + 1] = clamp_int4(vBy)

    # Left & right edge: 2x1 blocks
    for x in [0, 15]:
        for y in range(1, 15, 2):

            A = d[y][x]
            C = d[y + 1][x]

            vAx = vx[y][x]
            vAy = vy[y][x]

            vCx = vx[y + 1][x]
            vCy = vy[y + 1][x]

            ac = max_(0, vAy)  # A -> C
            ca = max_(0, -vCy)  # C -> A

            # 1. Source capacity
            # Each cell cannot give away more density than it contains.
            ac = min_(ac, A)
            ca = min_(ca, C)

            # 2. Destination capacity
            # Destination can hold at most MAX_DENSITY and
            # therefore receive at most MAX_DENSITY - <CELL DENSITY>
            ac = min_(ac, MAX_DENSITY - C)
            ca = min_(ca, MAX_DENSITY - A)

            # Apply transfers
            A += ca - ac
            C += ac - ca

            d[y][x] = A
            d[y + 1][x] = C

            if ac:
                dv = ac >> 2
                vAy -= dv
                vCy += dv

            if ca:
                dv = ca >> 2
                vCy -= dv
                vAy += dv

            vx[y][x] = clamp_int4(vAx)
            vy[y][x] = clamp_int4(vAy)

            vx[y + 1][x] = clamp_int4(vCx)
            vy[y + 1][x] = clamp_int4(vCy)

    # global friction_counter
    # friction_counter += 1

    # if friction_counter >= 4:
    #     friction_counter = 0

    #     # Apply friction
    #     for y in range(16):
    #         for x in range(16):
    #             vy[y][x] = apply_friction(vy[y][x])
    #             vx[y][x] = apply_friction(vx[y][x])

    s = 0
    for y in range(0, 16):
        s += sum(d[y])

    print("Total fluid: ", s)


def density_to_color(d):

    normalized = min_(d / MAX_DENSITY, 1.0)

    r = int(normalized * 255)
    g = int(normalized * 141)
    b = int(normalized * 54)

    return r, g, b


def draw(screen, font):
    screen.fill((10, 46, 2))

    for y in range(GRID_H):
        for x in range(GRID_W):

            d_ = d[y][x]

            screen_y = y * CELL_SIZE
            rect = pygame.Rect(x * CELL_SIZE, screen_y, CELL_SIZE, CELL_SIZE)

            # value = 0
            # if g_draw_velocity:
            # value = velocity_to_color(vy[y][x], vx[y][x])
            # else:
            value = density_to_color(d_)

            pygame.draw.rect(screen, value, rect)

            pygame.draw.rect(screen, (77, 43, 17), rect, 1)

            # A/B/C/D label
            block_x = x & 1
            block_y = y & 1

            if block_y == 0:
                label = "A" if block_x == 0 else "B"
            else:
                label = "C" if block_x == 0 else "D"

            label_text = font.render(label, True, (255, 255, 255))
            label_rect = label_text.get_rect(top=rect.top + 4, left=rect.left + 4)
            screen.blit(label_text, label_rect)

            # Density
            text = font.render(str(d_), True, (255, 255, 255))
            text_rect = text.get_rect(center=rect.center)
            screen.blit(text, text_rect)

            # vx and vy
            vx_text = font.render(f"vx: {vx[y][x]:.2f}", True, (255, 80, 80))
            vy_text = font.render(f"vy: {vy[y][x]:.2f}", True, (80, 180, 255))

            vx_rect = vx_text.get_rect(center=(rect.centerx, rect.bottom - 20))
            vy_rect = vy_text.get_rect(center=(rect.centerx, rect.bottom - 5))

            screen.blit(vx_text, vx_rect)
            screen.blit(vy_text, vy_rect)

    font = pygame.font.Font(None, 22)

    text = font.render(
        f"ax={g_acceleration[1]}, ay={g_acceleration[0]}", True, (255, 255, 255)
    )

    screen.blit(text, (0, 0))

    mouse_x, mouse_y = pygame.mouse.get_pos()

    center_x = WINDOW_W / 2
    center_y = WINDOW_H / 2

    dx = mouse_x - center_x
    dy = mouse_y - center_y

    # Faint circle showing acceleration magnitude
    radius = int(math.hypot(dx, dy))
    pygame.draw.circle(
        screen, (255, 255, 255, 40), (int(center_x), int(center_y)), radius, 2
    )

    # --------------------------------------------------------
    # Display gravity direction
    # --------------------------------------------------------

    gy, gx = g_acceleration

    center_x = WINDOW_W - 50
    center_y = 50

    arrow_length = 25

    end_x = center_x + gx * arrow_length
    end_y = center_y + gy * arrow_length

    pygame.draw.line(screen, (230, 230, 230), (center_x, center_y), (end_x, end_y), 3)

    pygame.draw.circle(screen, (230, 230, 230), (center_x, center_y), 4)


# ============================================================
# Mouse
# ============================================================


def add_fluid_at_mouse():
    mouse_x, mouse_y = pygame.mouse.get_pos()

    x = mouse_x // CELL_SIZE
    # y = mouse_y // CELL_SIZE

    y = GRID_H - 1 - (mouse_y // CELL_SIZE)

    if 0 <= x < GRID_W and 0 <= y < GRID_H:
        if d[y][x] + ADD_AMOUNT <= MAX_DENSITY:
            d[y][x] += ADD_AMOUNT


def handle_events():
    # ----------------------------------------------------
    # Events
    # ----------------------------------------------------

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            return False

        elif event.type == pygame.MOUSEBUTTONDOWN:

            if event.button == 1:
                add_fluid_at_mouse()

    # ----------------------------------------------------
    # Hold mouse button to continuously add fluid
    # ----------------------------------------------------

    if pygame.mouse.get_pressed()[0]:
        add_fluid_at_mouse()

    keys = pygame.key.get_pressed()

    global g_acceleration

    if keys[pygame.K_UP] and keys[pygame.K_LEFT]:
        g_acceleration = [-4, -4]
    elif keys[pygame.K_UP] and keys[pygame.K_RIGHT]:
        g_acceleration = [-4, 4]
    elif keys[pygame.K_DOWN] and keys[pygame.K_LEFT]:
        g_acceleration = [4, -4]
    elif keys[pygame.K_DOWN] and keys[pygame.K_RIGHT]:
        g_acceleration = [4, 4]
    elif keys[pygame.K_UP]:
        g_acceleration = [-4, 0]
    elif keys[pygame.K_DOWN]:
        g_acceleration = [4, 0]
    elif keys[pygame.K_LEFT]:
        g_acceleration = [0, -4]
    elif keys[pygame.K_RIGHT]:
        g_acceleration = [0, 4]

    mouse_x, mouse_y = pygame.mouse.get_pos()

    center_x = WINDOW_W / 2
    center_y = WINDOW_H / 2

    dx = mouse_x - center_x
    dy = mouse_y - center_y

    # 2 * CELL_SIZE is one unit
    g_acceleration = [
        int(clamp_velocity(dy / (2 * CELL_SIZE))),
        int(clamp_velocity(dx / (2 * CELL_SIZE))),
    ]

    return True


# ============================================================
# Main
# ============================================================


def main():

    pygame.init()

    screen = pygame.display.set_mode((WINDOW_W, WINDOW_H))
    pygame.display.set_caption("16x16 Fluid Simulation")

    clock = pygame.time.Clock()
    font = pygame.font.SysFont(None, 18)

    running = True

    WINDOW_SIZE = 1

    ax_history = deque(maxlen=WINDOW_SIZE)
    ay_history = deque(maxlen=WINDOW_SIZE)

    while running:
        global g_acceleration

        if not handle_events():
            break

        g_acceleration = [0, 0]

        print("Attempting to read line...")
        serial_buffer = b""

        if arduino.in_waiting:
            serial_buffer += arduino.read(arduino.in_waiting)

            while b"\n" in serial_buffer:
                raw_line, serial_buffer = serial_buffer.split(b"\n", 1)
                line = raw_line.decode("ascii", errors="ignore").strip()

                if not line or line == "0":
                    continue

                print(f"Line: {line}")

                try:
                    ay, ax = map(float, line.split(","))
                    c_ay, c_ax = scale_int16_to_int4(ay), scale_int16_to_int4(ax)
                    # g_acceleration = [c_ay, c_ax]

                    # Store the latest readings
                    ax_history.append(ax)
                    ay_history.append(ay)

                    # Calculate the averages of available samples
                    avg_ax = sum(ax_history) / len(ax_history)
                    avg_ay = sum(ay_history) / len(ay_history)

                    scaled_avg_ay = scale_int16_to_int4(int(avg_ay))
                    scaled_avg_ax = scale_int16_to_int4(int(avg_ax))

                    g_acceleration = [
                        scaled_avg_ay,
                        scaled_avg_ax
                    ]

                    print(f"Scaled: {c_ay},{c_ax}")
                    print(f"Average: ax={avg_ax:.2f}, ay={avg_ay:.2f}")
                    print(f"Avg scaled: {scaled_avg_ay},{scaled_avg_ax}")

                except ValueError:
                    pass

        # ----------------------------------------------------
        # Simulation
        # ----------------------------------------------------

        simulate()

        # ----------------------------------------------------
        # Rendering
        # ----------------------------------------------------

        draw(screen, font)
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == "__main__":
    main()

    # for i in range(0, NUM_SQUARES):
    #     y, x = to_yx(i)

    #     di = d[y][x]

    #     # Nothing to move
    #     if di == 0:
    #         continue

    #     vyi = vy[y][x]
    #     vxi = vx[y][x]

    #     avyi = abs(vyi)
    #     axyi = abs(vxi)

    #     fluid_to_be_moved = avyi + axyi

    #     # This is the case when cell won't get drained fully
    #     if di >= fluid_to_be_moved:
    #         move_y = avyi
    #         move_x = axyi

    #     # This is the case when a cell is fully drained
    #     # we have two options: 1. do nothing 2. transfer velocity to
    #     # neighbor. Sort of "keep momentum".
    #     else:
    #         # Not enough density: distribute all available density
    #         # proportionally between Y and X.
    #         move_y = (di * avyi + fluid_to_be_moved // 2) // fluid_to_be_moved
    #         move_x = di - move_y

    #     # Remove density from source
    #     d[y][x] -= move_y + move_x

    #     # Split fluid between y-direction neighbor
    #     ny = y + sign(vyi)
    #     if is_valid_y(ny):
    #         target_d = d[ny][x]

    #         space_left = MAX_DENSITY - target_d
    #         actual_move = min_(move_y, space_left)

    #         d[ny][x] += actual_move
    #         d[y][x] += move_y - actual_move
    #     else:
    #         # No destination -> keep the fluid
    #         d[y][x] += move_y

    #     # Split fluid between x-direction neighbor
    #     nx = x + sign(vxi)
    #     if is_valid_x(nx):
    #         target_d = d[y][nx]

    #         space_left = MAX_DENSITY - target_d
    #         actual_move = min_(move_x, space_left)

    #         d[y][nx] += actual_move
    #         d[y][x] += move_x - actual_move
    #     else:
    #         # No destination -> keep the fluid
    #         d[y][x] += move_x
