import pygame
from pygame.locals import *
import sys
import screen
import screens
import webbrowser



# Home screen
class HomeScreen(screen.Screen):

	# Constructor for home screen
	def __init__(self):

		# Current selection on menu
		self.selectMenu = 0

		# Font for the home screen
		pygame.font.init()
		self.font = pygame.font.SysFont('nimbusmonops', 20)

		# Home screen is not resizable
		self.resizable = False

		# Size of the window (not resizable so not negotiable)
		self.size = (1280, 700)



	# Build the background for home screen
	def generateBackround(self, background):

		# Fill background with dark blue
		BACKGROUND_COLOR = (23, 23, 178)
		background.fill(BACKGROUND_COLOR)

		# Top banner
		pygame.draw.line(background, (84, 255, 255), (20, 40), (1260, 40), 2)
		self.font.set_bold(True)
		img = self.font.render('st3sh - POSIX Terminal', True, (84, 255, 255))
		background.blit(img, (20, 13))

		# Center rectangles and shadows
		mainRect = pygame.Rect(40, 55, 1165, 590)
		pygame.draw.rect(background, (178, 178, 178), mainRect)

		shadowRectVert = pygame.Rect(1205, 90, 35, 590) 
		pygame.draw.rect(background, (0, 0, 0), shadowRectVert)

		shadowRectHorizontal = pygame.Rect(75, 645, 1130, 35)
		pygame.draw.rect(background, (0, 0, 0), shadowRectHorizontal)


		# Interior header text and border lines
		img = self.font.render('st3sh Start', True, (84, 84, 255))
		background.blit(img, (556, 60))

		pygame.draw.line(background, (255, 255, 255), (47, 69), (551, 69), 2)
		pygame.draw.line(background, (255, 255, 255), (694, 69), (1190, 69), 2)
		pygame.draw.line(background, (0, 0, 0), (1190, 69), (1198, 69), 2)
		pygame.draw.line(background, (255, 255, 255), (47, 69), (47, 625), 2)
		pygame.draw.line(background, (0, 0, 0), (1198, 69), (1198, 625), 2)
		pygame.draw.line(background, (255, 255, 255), (47, 625), (55, 625), 2)
		pygame.draw.line(background, (0, 0, 0), (55, 625), (1198, 625), 2)


		# Inner main text and box
		self.font.set_bold(False)

		img = self.font.render('Welcome to st3sh: Created by JohnDavid Abe.', True, (0, 0, 0))
		background.blit(img, (90, 90))
		img = self.font.render('Arrow keys navigate the menu. <Enter> selects items within the menu.', True, (0, 0, 0))
		background.blit(img, (90, 110))

		pygame.draw.line(background, (0, 0, 0), (90, 145), (1147, 145), 2)
		pygame.draw.line(background, (255, 255, 255), (1147, 145), (1155, 145), 2)
		pygame.draw.line(background, (0, 0, 0), (90, 145), (90, 600), 2)
		pygame.draw.line(background, (255, 255, 255), (1155, 145), (1155, 600), 2)
		pygame.draw.line(background, (0, 0, 0), (90, 600), (98, 600), 2)
		pygame.draw.line(background, (255, 255, 255), (98, 600), (1155, 600), 2)

		return background



	# Update the dynamic content for the home screen based on event
	def generateScreen(self, screen, event):

		# Check for key navigation through selection menu
		keys = pygame.key.get_pressed()
		if keys[pygame.K_DOWN]:
			self.selectMenu = min(5, self.selectMenu + 1)
		elif keys[pygame.K_UP]:
			self.selectMenu = max(1, self.selectMenu - 1)


		# Build the rectangle to highlight currently selected option
		highlightOptionRects = [pygame.Rect(185, 215, 214, 25),
						 pygame.Rect(185, 265, 262, 25),
						 pygame.Rect(185, 315, 94, 25),
						 pygame.Rect(185, 365, 106, 25),
						 pygame.Rect(185, 415, 94, 25)]

		if self.selectMenu != 0:
			pygame.draw.rect(screen, (23, 23, 178), highlightOptionRects[self.selectMenu - 1])


		# Display selection menu
		img = self.font.render('1. Start terminal', True, (255, 255, 255) if (self.selectMenu == 1) else (0, 0, 0))
		screen.blit(img, (190, 220))

		img = self.font.render('2. Open configuration', True, (255, 255, 255) if (self.selectMenu == 2) else (0, 0, 0))
		screen.blit(img, (190, 270))

		img = self.font.render('3. Docs', True, (255, 255, 255) if (self.selectMenu == 3) else (0, 0, 0))
		screen.blit(img, (190, 320))

		img = self.font.render('4. About', True, (255, 255, 255) if (self.selectMenu == 4) else (0, 0, 0))
		screen.blit(img, (190, 370))

		img = self.font.render('5. Exit', True, (255, 255, 255) if (self.selectMenu == 5) else (0, 0, 0))
		screen.blit(img, (190, 420))



	# Switch screen appropriately if selection is made within home menu
	def switchScreens(self, event):

		# Check if selection was made
		keys = pygame.key.get_pressed()
		if (keys[pygame.K_KP_ENTER] or keys[pygame.K_RETURN]) and self.selectMenu != 0:

			# Option 1, open terminal
			if self.selectMenu == 1:
				return screens.Screens.TERMINAL

			# Option 3, docs
			elif self.selectMenu == 3:
				# Just open documentation and return same screen
				webbrowser.open("https://github.com/JohnDCode/st3sh")
				return screens.Screens.HOME

			# Option 4, about
			elif self.selectMenu == 4:
				return screens.Screens.ABOUT

			# Option 5, exit
			elif self.selectMenu == 5:
				pygame.quit()
				sys.exit()


		# No change, return home screen
		return screens.Screens.HOME



	# Return that home screen is not resizable
	def getResizable(self):
		return self.resizable



	# Return the min (in this case as not resizable, absolute) size of home screen
	def getMinSize(self):
		return self.size
