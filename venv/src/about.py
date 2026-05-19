import pygame
from pygame.locals import *
import screen
import screens
import webbrowser



# About screen
class AboutScreen(screen.Screen):

	# Constructor for about screen
	def __init__(self):

		# Font for the about titles
		pygame.font.init()
		self.font = pygame.font.SysFont('nimbusmonops', 20)

		# Font for logo
		self.logoFont = pygame.font.SysFont('nimbusmonops', 10)

		# Font for about text
		self.aboutFont = pygame.font.SysFont('ubuntu', 19)

		# About screen is not resizable
		self.resizable = False

		# Size of the window (not resizable so not negotiable)
		self.size = (1280, 700)



	# Build the background for about screen
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
		img = self.font.render('st3sh About', True, (84, 84, 255))
		background.blit(img, (556, 60))
		self.font.set_bold(False)

		pygame.draw.line(background, (255, 255, 255), (47, 69), (551, 69), 2)
		pygame.draw.line(background, (255, 255, 255), (694, 69), (1190, 69), 2)
		pygame.draw.line(background, (0, 0, 0), (1190, 69), (1198, 69), 2)
		pygame.draw.line(background, (255, 255, 255), (47, 69), (47, 625), 2)
		pygame.draw.line(background, (0, 0, 0), (1198, 69), (1198, 625), 2)
		pygame.draw.line(background, (255, 255, 255), (47, 625), (55, 625), 2)
		pygame.draw.line(background, (0, 0, 0), (55, 625), (1198, 625), 2)


		# About text
		self.aboutFont.set_bold(True)
		img = self.aboutFont.render('Hi, I\'m John, a CS student at GATech!', True, (0, 0, 0))
		background.blit(img, (80, 99))

		self.aboutFont.set_bold(False)
		img = self.aboutFont.render('This is st3sh, an (almost) POSIX-compliant shell and terminal:', True, (0, 0, 0))
		background.blit(img, (80, 134))

		img = self.aboutFont.render('Shell is written in C17 and supports pipelines, separators', True, (0, 0, 0))
		background.blit(img, (120, 184))

		img = self.aboutFont.render('various redirect operators, etc. Terminal (UI) is written in', True, (0, 0, 0))
		background.blit(img, (120, 219))

		img = self.aboutFont.render('Python 3.12 using Pygame 2. Additional features coming soon!', True, (0, 0, 0))
		background.blit(img, (120, 254))

		img = self.aboutFont.render('To learn more about myself, this project, and others like it', True, (0, 0, 0))
		background.blit(img, (80, 319))

		img = self.aboutFont.render('check out the links below!', True, (0, 0, 0))
		background.blit(img, (80, 354))

		self.aboutFont.set_underline(True)
		img = self.aboutFont.render('github.com/JohnDCode/st3sh', True, (0, 0, 0))
		background.blit(img, (120, 404))

		img = self.aboutFont.render('johndcode.com/posts/About-Me', True, (0, 0, 0))
		background.blit(img, (120, 439))

		img = self.aboutFont.render('linkedin.com/in/johndcode', True, (0, 0, 0))
		background.blit(img, (120, 474))
		self.aboutFont.set_underline(False)


		

		# Display logo
		logoLines = [
		'                                               ,~.',
		'                                             ¿╜ N"w',
		'                                    ,,,,,,,,╨  ▐kk║',
		'                   ╓Tⁿ─~,    ,⌐*"`         \'"* ╠╗H]',
		'                   ▌ ╙,  `ⁿ`               ╓▓█ ^»▒j',
		'                   ╠  )ç\' ,                ▀²     ▒',
		'                   ▐ >▌² █▓▓                   M,w▒─^"```',
		'                    ╚µ╨           ,~r─t   ,⌐^,╛   É',
		'                     ▌  -K  ,,,w. ╙╜ÜP`  ⌐─ⁿP╙`▄å╙',
		'                    ,~æÇ` .Jm*╙å~  ,r*─¬⌐∞╨. ,Æ` `"^─¬w,',
		'     [ç         ⌐^`   ,Å╨╣▄*` `""`  -⌐─`    ,Å                ¿',
		'    ,;k ╖          ⌐^`,çåΦÑ▒▒m,          .`≤╢▒              ¿╜╓',
		'╓P╢O.,⌐─⌐▒▒,        ;▄ΩzHzzz░Ñ░▒▒D%mm╥m%╣▒▒Ñ░░░Rw         ,▄  ""ⁿ*g~,',
		'"*╨┴──╥æÑ░░░╢╥w╖Æ▒░z<<<<▌<z<<z<z░░░░░░░░░░░░░░zUH░hw,,   ▐░▌*`"^,JZM▒─',
		'       ╙▒░░░░░░░▒▒▒Wzz<«╢(<zzzzzz<z░░░░░░░zz<<)▒<z<<@▌▌Æ▒░░░▒╫╫╩``',
		'         ╙▒░░░░░░░░▒<<z<[▌<<zzzzzzzzzzzzzzzzz/@z<z<▒▒▒░░░░░░▒▀',
		'           ╙▒░░░░░░▒HNNÅ▒zzzzzzzzzzzzzzzzzzzz▐░/<)╢▒░░░░░░░▒',
		'              ²╙²`╙╜╜` ╓z<z<zzzzzzzzzzzzzzz</M²Ñ░H▒░░░░░▒╨',
		'                      ƒ<<zzzzzzzzzzzzzzzz<z<▐    `` `²└',
		'                     Æz<<zzzzzzzzzzzzzzzzz<<▌',
		'                    Æz<zzzzzzzzzzzzzzzzzzz<<▌                           ,gpm─',
		'                   ▐z<<zzzzzzzzzzzzzzzzzzz<<▌                       ¿$╨"',
		'                   ░<zzzzzzzzzzzzzzzzzzzzz<<╢                     rÑ`',
		'                  j%ù≥<<<z<<<<zzzzzzzz<zz<z<<w                  r¼`',
		'                   ▒▒▄▒░%æm⌂g//<<<«<z<<<«yzpm▒                ,Ö╨',
		'                  ╢▒▒▒▒▒▒▒▒▒╫╥▄▄▄▒▒▒▒▒▒▄▄▄╫╫▒▌               ƒ,╛',
		'                  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒             ,╛,"',
		'                  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒  ▒▒▒▒▒▒▒▒▒▒▒▒    ,r`,╜']

		for i in range(len(logoLines)):
			img = self.logoFont.render(logoLines[i], True, (0, 0, 0))
			background.blit(img, (700, 140 + (i * 13)))


		# Back button outline
		pygame.draw.line(background, (255, 255, 255), (1031, 578), (1173, 578), 2)
		pygame.draw.line(background, (0, 0, 0), (1173, 578), (1181, 578), 2)
		pygame.draw.line(background, (255, 255, 255), (1031, 578), (1031, 608), 2)
		pygame.draw.line(background, (255, 255, 255), (1031, 608), (1059, 608), 2)
		pygame.draw.line(background, (0, 0, 0), (1039, 608), (1181, 608), 2)
		pygame.draw.line(background, (0, 0, 0), (1181, 578), (1181, 608), 2)


		# Back button (little) shadows
		shadowRectVert = pygame.Rect(1183, 585, 7, 30) 
		pygame.draw.rect(background, (123, 123, 123), shadowRectVert)

		shadowRectHorizontal = pygame.Rect(1038, 610, 150, 7)
		pygame.draw.rect(background, (123, 123, 123), shadowRectHorizontal)


		return background



	# Update the dynamic content for the about screen based on event
	def generateScreen(self, screen, event):

		# Open links if mouse clicked and over any of the links
		pos = pygame.mouse.get_pos()


		# Github
		if pos[0] > 120 and pos[0] < 373 and pos[1] > 404 and pos[1] < 426:

			# Set mouse to select
			pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_HAND)

			# Check if clicked on link and open
			if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
				webbrowser.open("https://github.com/JohnDCode/st3sh")

		# Portfolio
		elif pos[0] > 120 and pos[0] < 394 and pos[1] > 439 and pos[1] < 461:

			pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_HAND)
			if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
				webbrowser.open("https://www.johndcode.com/posts/About-Me")

		# Linkedin
		elif pos[0] > 120 and pos[0] < 348 and pos[1] > 474 and pos[1] < 496:

			pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_HAND)
			if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
				webbrowser.open("https://www.linkedin.com/in/johndcode")

		# No selection
		else:
			pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_ARROW)

		img = self.font.render('Home (esc)', True, (0, 0, 0))
		screen.blit(img, (1051, 588))



	# Switch screen appropriately if user wants to go back to home
	def switchScreens(self, event):

		# Check if escaped back to home
		keys = pygame.key.get_pressed()
		if keys[pygame.K_ESCAPE]:
			return screens.Screens.HOME

		# No change, return about screen
		return screens.Screens.ABOUT



	# Return that about screen is not resizable
	def getResizable(self):
		return self.resizable



	# Return the min (in this case as not resizable, absolute) size of about screen
	def getMinSize(self):
		return self.size
