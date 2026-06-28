"""Pygame UI showing two hand buttons + foot-switch state."""
import pygame
from button_reader import ButtonReader
from foot_switch import FootSwitchMonitor

WIDTH, HEIGHT = 500, 300

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Coordination Kit — POC")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 28)

    br = ButtonReader()
    fs = FootSwitchMonitor()
    fs.start()

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        b0, b1 = br.read()
        color0 = (0, 255, 0) if b0 else (128, 128, 128)
        color1 = (0, 255, 0) if b1 else (128, 128, 128)
        foot_color = (0, 255, 0) if fs.pressed else (128, 128, 128)

        screen.fill((30, 30, 30))
        screen.blit(font.render(f"B0: {'ON' if b0 else 'OFF'}", True, color0), (50, 60))
        screen.blit(font.render(f"B1: {'ON' if b1 else 'OFF'}", True, color1), (50, 110))
        screen.blit(font.render(f"FOOT: {'ON' if fs.pressed else 'OFF'}", True, foot_color), (50, 160))
        pygame.display.flip()
        clock.tick(60)

    br.close()
    fs.stop()
    pygame.quit()

if __name__ == "__main__":
    main()
