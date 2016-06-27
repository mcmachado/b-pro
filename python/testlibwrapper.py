import os
import ctypes

path = os.getcwd() + '/visual_features.so'
visual_features = ctypes.CDLL(path)

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

param = Parameters(14, 16, 128)

print visual_features.getNumberOfFeatures(param.getNumRows(), param.getNumColumns(), param.getNumColors())


#visual_features.myprint()