import pygame
from pygame.locals import *
import screens
import shell
import time
import sys
import atexit



# Spawn shell proc
sh = shell.ShellThread("/home/john/st3sh2/build/st3sh")
sh.start()
time.sleep(0.05)

# Ensure shell gets killed on exit
def killShell():
	sh.stop()
atexit.register(killShell)


# Setup inital window and fonts
pygame.init()


# Track current screen
curScreen = screens.Screens.HOME
curSize = curScreen.value.getMinSize()


# Setup original screen
screen = pygame.display.set_mode(curSize)
background = curScreen.value.generateBackround(pygame.Surface(curSize))
pygame.display.set_caption('st3sh')
pygame.display.flip()
clock = pygame.time.Clock()



# Main event loop
running = True
while running:

	# Collect event, check for quit
	event = pygame.event.wait(timeout=50)
	if event.type == QUIT:
		running = False


	# Check if need to switch screens
	prevScreen = curScreen
	curScreen = curScreen.value.switchScreens(event)

	# If switched screens, update window details and background
	if prevScreen != curScreen:

		# Get min size of the screen
		minSize = curScreen.value.getMinSize()
		
		# If resizable, clamp window size to min size of new screen and set to resizable
		if curScreen.value.getResizable():
			curSize = (max(event.w, minSize[0]), max(event.h, minSize[1]))
			screen = pygame.display.set_mode(curSize, pygame.RESIZABLE)

		# If not resizable, window size becomes min size
		else:
			curSize = minSize
			screen = pygame.display.set_mode(curSize)


		# Get new background
		background = curScreen.value.generateBackround(pygame.Surface(curSize))
		event = pygame.event.Event(pygame.NOEVENT)


	# Check for resizing, clamp to min size of current screen
		# Note to self: recall non resizable screens can never generate this event anyways, no need to check
	if event.type == pygame.VIDEORESIZE:
		minSize = curScreen.value.getMinSize()

		if event.w < minSize[0] or event.h < minSize[1]:
			curSize = (max(event.w, minSize[0]), max(event.h, minSize[1]))
			screen = pygame.display.set_mode(curSize, pygame.RESIZABLE)

		else:
			curSize = (event.w, event.h)


	# Build static background content to the current screen
	screen.blit(background, (0, 0))


	# Build dyanmic content for the current screen
	curScreen.value.generateScreen(screen, event)


	# Update display
	pygame.display.flip()
