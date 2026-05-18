import pygame
from abc import ABC, abstractmethod
from typing import Self

# Interface for all screens
class Screen(ABC):

    # Generate static content for the screen
    @abstractmethod
    def generateBackround(self, background) -> pygame.Surface:
        pass

    # Generate dynamic content for the screen
    @abstractmethod
    def generateScreen(self, screen, event) -> None:
        pass

    # Check if need to switch off the current screen and return new screen
    @abstractmethod
    def switchScreens(self, event) -> Self:
        pass

    # Get resizability of the screen
    @abstractmethod
    def getResizable(self) -> bool:
        pass

    # Get the minimum (or absolute if not resizable) dimmensions of screen
    @abstractmethod
    def getMinSize(self) -> (int, int):
        pass
