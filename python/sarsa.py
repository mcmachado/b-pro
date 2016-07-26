'''
Author: Marlos C. Machado
'''

import sys
import time
import math
import random
import ctypes
import numpy as np
import scipy.sparse

from ale_python_interface import ALEInterface
from BPROSTFeatures import BPROST

epsilon = 0.01
alpha   = 0.10
lambd   = 0.90
gamma   = 0.99

numTrials          = 1
numIterationsInAlg = 1000
maxNumStepsPerIter = 18000

screenHeight = 210
screenWidth  = 160
numRows      = 14
numCols      = 16
numColors    = 128

class SarsaLambda:
	''' Implementation of Sarsa'''
	def __init__(self, env, representation, epsilon, alpha, lambd, gamma, optimism=0.0):
		# algorithm parameters
		self._alpha   = alpha
		self._gamma   = gamma
		self._lambda  = lambd
		self._epsilon = epsilon

		self._numActions     = len(env.getLegalActionSet())
		self._representation = representation
		self._numFeatures    = self._representation.getSizeActionSet()

		self._theta = np.zeros((self._numActions, self._numFeatures)) #scipy.sparse.lil_matrix((self._numActions, self._numFeatures))
		self._q     = np.zeros(self._numActions)
		self._qNext = np.zeros(self._numActions)

		self._optimism = optimism


	def _epsilonGreedy(self, qValues):
		a = random.choice(np.where(np.array(qValues) == np.array(qValues).max())[0])
		if random.uniform(0, 1) < self._epsilon:
			a = random.randint(0, self._numActions - 1)

		return a


	def _updateQValues(self, weights, phi):
		q = np.zeros(self._numActions)

		for a in xrange(self._numActions):
			q[a] = np.sum(weights[a][phi])
			#q[a] = np.dot(weights[a], phi)
		return q


	def _updateEligTraces(self, action, elig, phi):
		for a in xrange(self._numActions):
			elig[a] = self._gamma * self._lambda * elig[a]

		#This line breaks, obviously
		elig[action][phi] = np.maximum(elig[action][phi], 1)
		return elig

	def learnForOneEpisode(self, env, maxNumSteps):
		_elig       = np.zeros((self._numActions, self._numFeatures)) #scipy.sparse.lil_matrix((self._numActions, self._numFeatures))
		# we now get the current ALE screen and put it in a format acceptable by the C code
		pyScreen    = env.getScreen()
		screen      = pyScreen.ctypes.data_as(ctypes.POINTER(ctypes.c_int))
		self._representation.getActiveFeatures(screen)
		_phi = np.array(self._representation)
		#_phi        = np.zeros(self._representation.getSizeActionSet())
		#_phi[self._representation] = 1
		_phiNext    = []
		_nextAction = -1

		#self._q = self._updateQValues(self._theta, _phi)
		currentAction = self._epsilonGreedy(self._q)
		_cumulReward  = 0

		frameNumber = 0
		while frameNumber < maxNumSteps:
			#self._q = self._updateQValues(self._theta, _phi)

			reward = env.act(env.getLegalActionSet()[currentAction])
			_cumulReward += reward

			if not env.game_over():
				pyScreen = env.getScreen()
				screen   = pyScreen.ctypes.data_as(ctypes.POINTER(ctypes.c_int))
				self._representation.getActiveFeatures(screen)
				_phiNext = np.array(self._representation)
				#_phiNext = np.zeros(self._representation.getSizeActionSet())
				#_phiNext[self._representation] = 1
				self._qNext = self._updateQValues(self._theta, _phiNext)
				_nextAction = self._epsilonGreedy(self._qNext)
			else:
				self._qNext = np.zeros(self._numActions)

			delta = reward + self._gamma * (self._qNext[_nextAction] + self._optimism) - (self._q[currentAction] + self._optimism)
			_elig = self._updateEligTraces(currentAction, _elig, _phi)

			for i in xrange(self._numActions):
				self._theta[i] = self._theta[i] + self._alpha * delta * _elig[i]

			_phi = _phiNext
			currentAction = _nextAction

			frameNumber += 5

		return _cumulReward

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

	# we instantiate the class implementing the desired representation
	#bpros = BPROS(screenHeight, screenWidth, numRows, numCols, numColors)
	bprost = BPROST(screenHeight, screenWidth, numRows, numCols, numColors)

	# this is how you get the total number of features:
	print 'Size of action set:', bprost.getSizeActionSet()

	for s in xrange(numTrials):
		random.seed(s + 1)
		sarsa = SarsaLambda(ale, bprost, epsilon, alpha, lambd, gamma)
		start_time = time.time()
		# for each iteration in the algorithm:
		for i in xrange(numIterationsInAlg):
			reward = sarsa.learnForOneEpisode(ale, maxNumStepsPerIter)
			total_reward += reward

		ale.reset_game()
		# for performance purposes we keep track of frame rate
		elapsed_time = time.time() - start_time
		fps = (ale.getFrameNumber() - previousFrameCount)/elapsed_time
		previousFrameCount = ale.getFrameNumber()
		print 'Episode %d ended with score %d running at %d fps.' % (episode + 1, total_reward, fps)

if __name__ == "__main__":
    main()