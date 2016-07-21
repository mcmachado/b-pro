''' Simple example for using the functions that implement B-PROS, B-PROST and Blob-PROST
    features. This implementation is supposed to show how one can use the invoke the code
    provided in C using ctypes. The implemented feature sets were originally introduced in
    the paper below. 

    Yitao Liang, Marlos C. Machado, Erik Talvitie, Michael H. Bowling:
    State of the Art Control of Atari Games Using Shallow Reinforcement Learning. 
    AAMAS 2016: 485-493

    Author: Marlos C. Machado
'''

# required imports
import sys
import time
import random
import ctypes

import numpy as np

from BPROSFeatures import BPROS
from ale_python_interface import ALEInterface

# global variables that encode the representation parameters
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
	ale.setInt(b'frame_skip', 5)
	# load rom provided in the command line
	rom_file = str.encode(sys.argv[1])
	ale.loadROM(rom_file)
	# get the list of legal actions
	legal_actions = ale.getLegalActionSet()

	# we instantiate the class implementing the desired representation
	bpros = BPROS(screenHeight, screenWidth, numRows, numCols, numColors)

	# this is how you get the total number of features:
	print 'Size of action set:', bpros.getSizeActionSet()

	# we play the game, requesting the screen at each iteration.
	previousFrameCount = 0
	for episode in xrange(5):
		total_reward = 0
		start_time = time.time()
		while not ale.game_over():
			# we now get the current ALE screen and put it in a format acceptable by the C code
			pyScreen = ale.getScreen()
			screen = (ctypes.c_int * len(pyScreen))()
			# we finally call the function that stores the feature vector inside the object
			bpros.getActiveFeatures(screen)
			#print len(bpros)
			# we randomly select an action in the environment to observe the next state
			a = legal_actions[random.randrange(len(legal_actions))]
			reward = ale.act(a);
			total_reward += reward

		# for performance purposes we keep track of frame rate
		elapsed_time = time.time() - start_time
		fps = (ale.getFrameNumber() - previousFrameCount)/elapsed_time
		previousFrameCount = ale.getFrameNumber()
		print 'Episode %d ended with score %d running at %d fps.' % (episode + 1, total_reward, fps)
		ale.reset_game()


if __name__ == "__main__":
	main()