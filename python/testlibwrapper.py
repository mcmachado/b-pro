''' Simple example for using the functions that implement B-PROS, B-PROST and Blob-PROST
    features. This implementation is supposed to show how one can use the invoke the code
    provided in C using ctypes. The implemented feature sets were originally introduced in
    the paper below. 

    Yitao Liang, Marlos C. Machado, Erik Talvitie, Michael H. Bowling:
    State of the Art Control of Atari Games Using Shallow Reinforcement Learning. 
    AAMAS 2016: 485-493

    Author: Marlos C. Machado
'''

# Required imports
import os
import sys
import ctypes

import numpy as np
from random import randrange

from ale_python_interface import ALEInterface

# Ctypes part
path = os.getcwd() + '/visual_features.so'
visual_features = ctypes.CDLL(path)

# Global variables that encode the representation parameters
numRows      = 14
numCols      = 16
numColors    = 128
screenHeight = 210
screenWidth  = 160


def main():
	# just a simple check first, we really need this rom file ;)
	if len(sys.argv) < 2:
  		print('Usage: %s rom_file' % sys.argv[0])
  		sys.exit()
	
	# Arcade Learning Environment
	ale      = ALEInterface()
	ale.setInt(b'random_seed', 123)
	# load rom provided in the command line
	rom_file = str.encode(sys.argv[1])
	ale.loadROM(rom_file)
	# get the list of legal actions
	legal_actions = ale.getLegalActionSet()

	#This is how you get the total number of features:
	print 'Max. number of features:', \
		visual_features.getNumberOfFeatures(numRows, numCols, numColors)

	# We now get the current ALE screen and put it in a format acceptable by the C code
	pyScreen = ale.getScreen().astype(int)
	screen = (ctypes.c_char_p * len(pyScreen))()
	screen[:] = pyScreen

	# We finally call the function that returns the feature set
	visual_features.getBROSFeatures(screen, screenHeight, screenWidth, numRows, numCols, numColors)

	# We play the game once. The code above shows how we can use the function, 
	# but in the loop we can check how fast this implementation is (TODO)
	total_reward = 0
	while not ale.game_over():
		a = legal_actions[randrange(len(legal_actions))]
		# Apply an action and get the resulting reward
		reward = ale.act(a);
		total_reward += reward

	print('Episode ended with score: %d' % total_reward)
	ale.reset_game()


if __name__ == "__main__":
	main()