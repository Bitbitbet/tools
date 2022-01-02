#!/usr/bin/env python3
import pygame
pygame.init()
screen = pygame.display.set_mode((600, 600))
pygame.display.set_caption('RGB熬')
font = pygame.font.SysFont(pygame.font.get_default_font(), 48)
r = 0
g = 0
b = 0
now = 0
w = screen.get_width()/2
h = screen.get_height()/2
while True:
    screen.fill((r, g, b))
    text = font.render('the United States of America',
                       True, (255-r, 255-g, 255-b))
    screen.blit(text, (w-text.get_width()/2, h-text.get_height()/2))
    pygame.display.update()
    if now == 0:
        r += 1
        if r == 256:
            now = 1
            r = 255
            b = 0
            g += 1
        if b > 0:
            b -= 1
    if now == 1:
        g += 1
        if g == 256:
            now = 2
            g = 255
            r = 0
            b += 1
        if r > 0:
            r -= 1
    if now == 2:
        b += 1
        if b == 256:
            now = 0
            b = 255
            g = 0
            r += 1
        if g > 0:
            g -= 1
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()
