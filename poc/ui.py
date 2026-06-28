"""Pygame UI showing two hand buttons + two foot switches (2=left, 4=right)."""
import pygame
from button_reader import ButtonReader

WIDTH, HEIGHT = 500, 300

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Coordination Kit — POC")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 28)
    br = ButtonReader()

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        keys = pygame.key.get_pressed()
        fl = keys[pygame.K_2]
        fr = keys[pygame.K_4]

        b0, b1 = br.read()
        screen.fill((30, 30, 30))
        screen.blit(font.render(f"B0: {'ON' if b0 else 'OFF'}", True, (0,255,0) if b0 else (128,128,128)), (50, 60))
        screen.blit(font.render(f"B1: {'ON' if b1 else 'OFF'}", True, (0,255,0) if b1 else (128,128,128)), (50, 110))
        screen.blit(font.render(f"FL: {'ON' if fl else 'OFF'}", True, (0,255,0) if fl else (128,128,128)), (50, 160))
        screen.blit(font.render(f"FR: {'ON' if fr else 'OFF'}", True, (0,255,0) if fr else (128,128,128)), (50, 210))
        pygame.display.flip()
        clock.tick(60)

    br.close()
    pygame.quit()

if __name__ == "__main__":
    main()
