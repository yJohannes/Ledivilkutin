import pygame

from config import SIM_FPS, WINDOW_W, WINDOW_H
from initialization import create_particles
from renderer import render
from simulation import Simulation
from serial_controller import SerialController

p, pp = create_particles()
sim = Simulation(p, pp)
controller = SerialController()

# Timer to let the start animation play
start_time = pygame.time.get_ticks()
acceleration_delay = 4000


def handle_events():

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            return False

    return True


def main():

    pygame.init()

    screen = pygame.display.set_mode((WINDOW_W, WINDOW_H))
    pygame.display.set_caption("16x16 Fluid Simulation")

    clock = pygame.time.Clock()
    font = pygame.font.SysFont(None, 18)

    running = True

    while running:

        running = handle_events()

        if not running:
            break

        acceleration = controller.read_acceleration()

        elapsed = pygame.time.get_ticks() - start_time

        if elapsed >= acceleration_delay and acceleration is not None:
            sim.set_acceleration(acceleration)

        sim.simulation_step()

        render(screen, font, sim)

        pygame.display.flip()
        clock.tick(SIM_FPS)

    controller.close()

    pygame.quit()


if __name__ == "__main__":
    main()
