from vec2 import Vec2
from config import (
    SIM_PARTICLE_COUNT,
    SIM_PARTICLE_MAX_DISPLACEMENT,
    SIM_PARTICLE_MIN_DIST,
    SIM_PARTICLE_RADIUS,
    SIM_PARTICLE_RADIUS_SQ,
    SIM_COM_EFFECTIVE_RADIUS_SQ,
    SIM_USE_EUCLIDEAN_SEPARATION,
)


class Simulation:
    def __init__(self, p, pp):
        self.particle_count = SIM_PARTICLE_COUNT
        self.acceleration = Vec2(0, 0)
        self.frame_counter = 0
        self.p = p  # Reference to arrays
        self.pp = pp

        # Two uint16 for center of mass calculations.
        # Positions are accumulated and at the end of frame
        # Division is performed to get COM.
        # Can't overflow because we reach at max 75 * 255 = 19125.
        self.com_u16 = Vec2(0, 0)

    def set_acceleration(self, a: Vec2):
        self.acceleration = a

    def simulate_frame(self):
        p = self.p
        pp = self.pp
        self.frame_counter += 1

        # Apply acceleration every ?? frame
        if self.frame_counter % 3 == 0:
            a: Vec2 = self.acceleration
        else:
            a = Vec2(0, 0)

        # Save old pos and
        for i in range(SIM_PARTICLE_COUNT):
            x = p[i].x
            y = p[i].y

            # # Signed implicit velocity.
            dx = x - pp[i].x
            dy = y - pp[i].y

            dx = max(
                -SIM_PARTICLE_MAX_DISPLACEMENT,
                min(SIM_PARTICLE_MAX_DISPLACEMENT, dx)
            )
            dy = max(
                -SIM_PARTICLE_MAX_DISPLACEMENT,
                min(SIM_PARTICLE_MAX_DISPLACEMENT, dy)
            )

            nx = x + dx + a.x
            ny = y + dy + a.y

            # Save previous position before overwriting current.
            pp[i].x = x
            pp[i].y = y

            # Reject movement that would cross a boundary.
            if nx < 0 or nx > 255:
                nx = x

            if ny < 0 or ny > 255:
                ny = y

            # Commit the updated position.
            p[i].x = nx
            p[i].y = ny

        for i in range(SIM_PARTICLE_COUNT):
            pi = p[i]

            # i + 1 because both particles are updated.
            for j in range(i + 1, SIM_PARTICLE_COUNT):
            # for j in range(SIM_PARTICLE_COUNT):

                # if i == j:
                    # continue

                self.separate_pair(i, j)

        # Calculate COM
        if self.frame_counter % 5 == 2:
            self.com_u16 = Vec2(0, 0)

            for i in range(SIM_PARTICLE_COUNT):
                self.com_u16.x += p[i].x
                self.com_u16.y += p[i].y

            com_x = self.com_u16.x // SIM_PARTICLE_COUNT
            com_y = self.com_u16.y // SIM_PARTICLE_COUNT

            for i in range(SIM_PARTICLE_COUNT):
                pi = p[i]

                dx = com_x - pi.x
                dy = com_y - pi.y

                # if dx * dx + dy * dy <= SIM_COM_EFFECTIVE_RADIUS_SQ:
                #     continue

                if dx > 0:
                    pi.x += 1
                elif dx < 0:
                    pi.x -= 1

                if dy > 0:
                    pi.y += 1
                elif dy < 0:
                    pi.y -= 1

        # Apply friction
        if self.frame_counter % 6 == 5:
            for i in range(SIM_PARTICLE_COUNT):
                pi = p[i]
                ppi = pp[i]

                dx = pi.x - ppi.x
                dy = pi.y - ppi.y

                # Reduce velocity magnitude by 1, preserving direction.
                if dx > 0:
                    ppi.x += 1
                elif dx < 0:
                    ppi.x -= 1

                if dy > 0:
                    ppi.y += 1
                elif dy < 0:
                    ppi.y -= 1

    def separate_pair(self, i, j):
        p = self.p

        dx = int(p[j].x) - int(p[i].x)
        dy = int(p[j].y) - int(p[i].y)

        if SIM_USE_EUCLIDEAN_SEPARATION:
            min_dist = SIM_PARTICLE_MIN_DIST
            dist_sq = dx * dx + dy * dy

            if dist_sq >= min_dist * min_dist:
                return

            # Choose a deterministic direction if the particles coincide.
            if dist_sq == 0:
                dx, dy = 1, 0
                dist_sq = 1

            dist = dist_sq ** 0.5
            overlap = min_dist - dist

            # Split the correction between both particles.
            correction_x = dx * overlap / (2 * dist)
            correction_y = dy * overlap / (2 * dist)

            p[i].x = max(0, min(255, round(p[i].x - correction_x)))
            p[i].y = max(0, min(255, round(p[i].y - correction_y)))

            p[j].x = max(0, min(255, round(p[j].x + correction_x)))
            p[j].y = max(0, min(255, round(p[j].y + correction_y)))

        else:
            ax = abs(dx)
            ay = abs(dy)

            # Already separated along at least one axis
            if ax >= SIM_PARTICLE_MIN_DIST or \
               ay >= SIM_PARTICLE_MIN_DIST:
                return

            # Choose the axis with the smaller overlap correction
            if ax >= ay:
                overlap = SIM_PARTICLE_MIN_DIST - ax
                correction = overlap >> 1

                if dx > 0:
                    p[i].x = max(0, p[i].x - correction)
                    p[j].x = min(255, p[j].x + (overlap - correction))
                elif dx < 0:
                    p[i].x = min(255, p[i].x + correction)
                    p[j].x = max(0, p[j].x - (overlap - correction))
                else:
                    # Coincident on x: choose a deterministic direction
                    p[i].x = max(0, p[i].x - correction)
                    p[j].x = min(255, p[j].x + (overlap - correction))

            else:
                overlap = SIM_PARTICLE_MIN_DIST - ay
                correction = overlap >> 1

                if dy > 0:
                    p[i].y = max(0, p[i].y - correction)
                    p[j].y = min(255, p[j].y + (overlap - correction))
                elif dy < 0:
                    p[i].y = min(255, p[i].y + correction)
                    p[j].y = max(0, p[j].y - (overlap - correction))
                else:
                    p[i].y = max(0, p[i].y - correction)
                    p[j].y = min(255, p[j].y + (overlap - correction))
