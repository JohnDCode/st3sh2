import pygame
from pygame.locals import *
import screen
import screens
import subprocess
import pty
import time
import os
import fcntl
import shell
import queue


# Terminal screen
class TerminalScreen(screen.Screen):

    # Constructor for home screen
    def __init__(self):

        # Font for the banners on the terminal screen
        pygame.font.init()
        self.font = pygame.font.SysFont('nimbusmonops', 20)

        # Terminal screen is not resizable (for now)
        self.resizable = False

        # Size of the window (not resizable so not negotiable)
        self.size = (1280, 700)

        # Buffer recieving from shell output
        self.buffer = "st3sh\n\nShift+Esc to return home\n\n"


    # Build the background for terminal screen
    def generateBackround(self, background):

        # Reset buffer
        self.buffer = "st3sh\n\nShift+Esc to return home\n\n"

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
        img = self.font.render('st3sh', True, (84, 84, 255))
        background.blit(img, (591, 60))

        pygame.draw.line(background, (255, 255, 255), (47, 69), (586, 69), 2)
        pygame.draw.line(background, (255, 255, 255), (659, 69), (1190, 69), 2)
        pygame.draw.line(background, (0, 0, 0), (1190, 69), (1198, 69), 2)
        pygame.draw.line(background, (255, 255, 255), (47, 69), (47, 625), 2)
        pygame.draw.line(background, (0, 0, 0), (1198, 69), (1198, 625), 2)
        pygame.draw.line(background, (255, 255, 255), (47, 625), (55, 625), 2)
        pygame.draw.line(background, (0, 0, 0), (55, 625), (1198, 625), 2)


        # Main interior with terminal content
        self.textBox = pygame.Rect(67, 89, 1111, 516)
        self.scrollWidth = 12
        self.padding = 10

        # Section of text in main view that is visible
        self.viewPort = pygame.Rect(
            self.textBox.x,
            self.textBox.y,
            self.textBox.width - self.scrollWidth,
            self.textBox.height
        )

        # Scroll bar
        self.scrollBar = pygame.Rect(
            self.viewPort.right,
            self.textBox.y,
            self.scrollWidth,
            self.textBox.height
        )

        # Build inital content
        self.buildTextContent()


        # Scrolling mechanics
        self.scrollSpeed = 30
        self.scrollY = 0
        self.dragging = False
        self.dragOffset = 0

        return background



    # Update the dynamic content for the home screen based on event
    def generateScreen(self, screen, event):

        # Read the output from the shell
        try:
            while True:
                output = shell.shellOutput.get_nowait()
                self.buffer += output.decode(errors="replace")
                self.buildTextContent()
        except queue.Empty:
            pass


        # Check for user input, pass to shell via queue
        if event.type == pygame.KEYDOWN:

            #  Put user input onto the qeueue for pty to write to shell
            shell.shellInput.put(event.unicode.encode("utf-8"))


        # Check for user scrolling / moving view
        elif event.type == pygame.MOUSEWHEEL:
            self.scrollY -= event.y * self.scrollSpeed
        self.scrollY = max(0, min(self.scrollY, self.maxScroll))

        # Box background
        pygame.draw.rect(screen, (45, 45, 45), self.textBox)

        # Clip
        screen.set_clip(self.viewPort)
        screen.blit(self.contentSurface, (self.viewPort.x, self.viewPort.y - self.scrollY))
        screen.set_clip(None)

        # Scrollbar track
        pygame.draw.rect(screen, (35, 35, 35), self.scrollBar)

        # Scrollbar thumb
        thumbBox = self.buildThumb()
        pygame.draw.rect(screen, (140, 140, 140), thumbBox, border_radius=4)

        # Borders
        pygame.draw.rect(screen, (120, 120, 120), self.textBox, 2)



    # Build the thumb based on current position from scroll
    def buildThumb(self):

        # Check if the view is even full of content (needs a thumb)
        if self.contentHeight <= self.viewPort.height:
            return pygame.Rect(self.scrollBar.x, self.scrollBar.y, self.scrollWidth, self.scrollBar.height)

        # If content occupies more space than view, build thumb to scroll
        ratio = self.viewPort.height / self.contentHeight
        thumbHeight = max(30, ratio * self.scrollBar.height)

        thumbY = self.scrollBar.y + (self.scrollY / self.maxScroll) * (self.scrollBar.height - thumbHeight)
        return pygame.Rect(self.scrollBar.x, thumbY, self.scrollWidth, thumbHeight)



    # Build the visible content from self.content to the viewport screen
    def buildTextContent(self):

        # Split up new lines
        lines = self.buffer.split("\n")

        # Wrap lines to fit in the text box window
        displayLines = []
        for line in lines:

            # Current line broken down to fit / wrap within the window
            curLine = []
            
            # Blank line
            if line == "":
                displayLines.append("")
                continue

            # Get the width each line can fit on
            maxWidth = self.viewPort.width

            # Check if this line can fit on the width (no wrap)
            if self.font.size(line)[0] <= maxWidth:
                displayLines.append(line)
                continue

            
            # Split line by whitespace
            words = line.split(" ")

            # Build a line by words until exceeds the width of the view
            cur = ""
            for w in words:

                # Add another word to the current line
                testStr = cur + (" " if cur else "") + w

                # Check if exceeds the line
                if font.size.size(testStr)[0] <= maxWidth:
                    cur = testStr
                elif cur:
                        displayLines.append(cur)
                        cur = w

            # Add running line
            displayLines.append(cur)



        # Height of all the content (lines)
        self.contentHeight = len(displayLines) * self.font.get_height() + self.padding * 2

        # Build content to the viewport screen
        self.contentSurface = pygame.Surface((self.viewPort.width, self.contentHeight))
        self.contentSurface.fill((30, 30, 30))

        y = self.padding
        for line in displayLines:
            self.contentSurface.blit(
                self.font.render(line, True, (220, 220, 220)),
                (self.padding, y)
            )
            y += self.font.get_height()


        self.maxScroll = max(1, self.contentHeight - self.viewPort.height)



    # Switch screen appropriately if selection is made within home menu
    def switchScreens(self, event):

        # Check if escaped back to home (shift esc)
        keys = pygame.key.get_pressed()
        if keys[pygame.K_ESCAPE] and (keys[pygame.K_RSHIFT] or keys[pygame.K_LSHIFT]):
            return screens.Screens.HOME

        # No change, stay on terminal
        return screens.Screens.TERMINAL



    # Return that home screen is not resizable
    def getResizable(self):
        return self.resizable



    # Return the min (in this case as not resizable, absolute) size of home screen
    def getMinSize(self):
        return self.size