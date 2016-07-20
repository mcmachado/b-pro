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

# Global variables that encode the representation parameters
numRows      = 14
numCols      = 16
numColors    = 128
screenHeight = 210
screenWidth  = 160

class Vector(object):
    path = os.getcwd() + '/visual_features.so'
    visual_features = ctypes.CDLL(path)
    visual_features.new_vector.restype = ctypes.c_void_p
    visual_features.new_vector.argtypes = []
    visual_features.delete_vector.restype = None
    visual_features.delete_vector.argtypes = [ctypes.c_void_p]
    visual_features.vector_size.restype = ctypes.c_int
    visual_features.vector_size.argtypes = [ctypes.c_void_p]
    visual_features.vector_get.restype = ctypes.c_int
    visual_features.vector_get.argtypes = [ctypes.c_void_p, ctypes.c_int]
    visual_features.vector_push_back.restype = None
    visual_features.vector_push_back.argtypes = [ctypes.c_void_p, ctypes.c_int]
    visual_features.getBROSFeatures.restype = None
    visual_features.getBROSFeatures.argtypes = [ctypes.c_void_p, (ctypes.c_int * 33600), \
    	ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]

    def __init__(self):
        self.vector = Vector.visual_features.new_vector()  # pointer to new vector

    def __del__(self):  # when reference count hits 0 in Python,
        Vector.visual_features.delete_vector(self.vector)  # call C++ vector destructor

    def __len__(self):
        return Vector.visual_features.vector_size(self.vector)

    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return Vector.visual_features.vector_get(self.vector, ctypes.c_int(i))
        raise IndexError('Vector index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    def push(self, i):  # push calls vector's push_back
        Vector.visual_features.vector_push_back(self.vector, ctypes.c_int(i))

    def getBROSFeatures(self, pyScreen, screenHeight, screenWidth, numRows, numCols, numColors):
    	screen = (ctypes.c_int * len(pyScreen))()

    	for i in xrange(len(pyScreen)):
    		screen[i] = pyScreen[i]

        Vector.visual_features.getBROSFeatures(self.vector, screen, 
        	ctypes.c_int(screenHeight), ctypes.c_int(screenWidth), ctypes.c_int(numRows),
        	ctypes.c_int(numCols), ctypes.c_int(numColors))

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
	#print 'Max. number of features:', \
	#	visual_features.getNumberOfFeatures(numRows, numCols, numColors)

	# We now get the current ALE screen and put it in a format acceptable by the C code
	pyScreen = ale.getScreen().tolist()

	feat = Vector()
	# We finally call the function that returns the feature set
	feat.getBROSFeatures(pyScreen, screenHeight, screenWidth, numRows, numCols, numColors)

	print feat

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