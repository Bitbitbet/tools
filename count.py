import pygame,time
pygame.init()
screen=pygame.display.set_mode(flags=pygame.FULLSCREEN)
font=pygame.font.SysFont(pygame.font.get_default_font(),160)
hour=0
minute=0
second=0
w=screen.get_width()/2
h=screen.get_height()/2
screen.fill((0,135,255))
pygame.display.update()
while True:
    text="%d:"%(hour)
    if minute<10:text=text+"0"
    text="%s%d:"%(text,minute)
    if second<10:text=text+"0"
    text="%s%d"%(text,second)
    screen.fill((0,135,255))
    text=font.render(text,True,(0,0,0))
    screen.blit(text,(w-text.get_width()/2,h-text.get_height()/2))
    pygame.display.update()
    time.sleep(1)
    second+=1
    if second==60:
        minute+=1
        second=0
        if minute==60:
            minute=0
            hour+=1

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()
