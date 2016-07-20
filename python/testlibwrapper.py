import os
import sys
import ctypes

import numpy as np
from random import randrange

from ale_python_interface import ALEInterface

np.set_printoptions(threshold=np.inf)

path = os.getcwd() + '/visual_features.so'
visual_features = ctypes.CDLL(path)

screenHeight = 210
screenWidth  = 160

class Parameters:
	def __init__(self, pRows = 14, pColumns = 16, pColors = 128):
		self.numRows    = pRows
		self.numColumns = pColumns
		self.numColors  = pColors

	def getNumRows(self):
		return self.numRows

	def getNumColumns(self):
		return self.numColumns

	def getNumColors(self):
		return self.numColors


def main():
	# just a simple check first
	if len(sys.argv) < 2:
  		print('Usage: %s rom_file' % sys.argv[0])
  		sys.exit()

  	# this is just to simplify the code
	param = Parameters(14, 16, 128)
	
	# Arcade Learning Environment
	ale      = ALEInterface()
	ale.setInt(b'random_seed', 123)
	# load rom provided in the command line
	rom_file = str.encode(sys.argv[1])
	ale.loadROM(rom_file)
	# get the list of legal actions
	legal_actions = ale.getLegalActionSet()

	print visual_features.getNumberOfFeatures(param.getNumRows(), 
		param.getNumColumns(), param.getNumColors())

	pyScreen = ale.getScreen().astype(int)

	screen = (ctypes.c_char_p * len(pyScreen))()
	screen[:] = pyScreen

	for i in xrange(screenHeight):
		for j in xrange(screenWidth):
			pixel = visual_features.getPixel(i, j, 
				screen, screenHeight, screenWidth)
			if pixel != 0:
				print i, j, pixel

	#visual_features.getBROSFeatures(screen, screenHeight, screenWidth, 
	#	param.getNumRows(), param.getNumColumns(), param.getNumColors())

	# we play the game once, we need data (screens)
	# to call the functions we are interested at
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