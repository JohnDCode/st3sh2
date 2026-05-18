from enum import Enum
import home
import about
import terminal

# Enumeration for all screens
class Screens(Enum):

	# Home screen with selection menu
	HOME = home.HomeScreen()
	ABOUT = about.AboutScreen()
	TERMINAL = terminal.TerminalScreen()
