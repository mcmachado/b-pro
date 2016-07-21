''' CTypes interface to the B-PROS feature set implementation, which was
    introduced in the paper below.

    Yitao Liang, Marlos C. Machado, Erik Talvitie, Michael H. Bowling:
    State of the Art Control of Atari Games Using Shallow Reinforcement Learning. 
    AAMAS 2016: 485-493

    Author: Marlos C. Machado
'''

import os
import ctypes

# This implementation is a little bit tricky because I wanted to receive a vector,
# passed by reference from the C++ code. To do so, I had to redefine several vector
# functions. Still, ideally the unique functions that should be used explicitly are:
#    getSizeActionSet(self)
#    getActiveFeatures(self, screen)

class BPROS(object):
    # loading C++ code using CTypes
    path = os.getcwd() + '/visual_features.so'
    bpros_features = ctypes.CDLL(path)
    # args and return types for constructor
    bpros_features.new_vector.restype = ctypes.c_void_p
    bpros_features.new_vector.argtypes = []
    # args and return types for destructor
    bpros_features.delete_vector.restype = None
    bpros_features.delete_vector.argtypes = [ctypes.c_void_p]
    # args and return types for clear function
    bpros_features.clear_vector.restype = None
    bpros_features.clear_vector.argtypes = [ctypes.c_void_p]
    # args and return types for size function
    bpros_features.vector_size.restype = ctypes.c_int
    bpros_features.vector_size.argtypes = [ctypes.c_void_p]
    # args and return types for get function ([] operator)
    bpros_features.vector_get.restype = ctypes.c_int
    bpros_features.vector_get.argtypes = [ctypes.c_void_p, ctypes.c_int]
    # args and return types for the functions we really care about
    bpros_features.getBROSFeatures.restype = None
    bpros_features.getBROSFeatures.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int), \
    	ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
    bpros_features.getNumberOfFeatures.restype = ctypes.c_int
    bpros_features.getNumberOfFeatures.argtypes = [ctypes.c_void_p]

    # these arguments are initialized by the constructor and they are kept fixed
    _screen_height = -1
    _screen_width  = -1
    _num_colors    = -1
    _num_rows      = -1
    _num_cols      = -1

    # C++ constructor and initialization of Python variables
    def __init__(self, screenHeight, screenWidth, numRows, numCols, numColors):
        self._screen_height = screenHeight
        self._screen_width  = screenWidth
        self._num_colors    = numColors
        self._num_rows      = numRows
        self._num_cols      = numCols

        self.vector = BPROS.bpros_features.new_vector()  # pointer to new vector

    # C++ destructor
    def __del__(self):  # when reference count hits 0 in Python,
        BPROS.bpros_features.delete_vector(self.vector)  # call C++ vector destructor

    # C++ size method
    def __len__(self):
        return BPROS.bpros_features.vector_size(self.vector)

    # C++ get method, which is the same as the [] operator
    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return BPROS.bpros_features.vector_get(self.vector, ctypes.c_int(i))
        raise IndexError('Vector index out of range')

    # C++ method that allows us to properly print a vector
    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    # C++ clear method
    def _clear(self):  # we can clear the vector without deleting it
        BPROS.bpros_features.clear_vector(self.vector)  # call C++ vector clear

    # C++ implementation for obtaining B-PROS features from a screen
    def getActiveFeatures(self, screen):
        self._clear()
        BPROS.bpros_features.getBROSFeatures(self.vector, screen, 
        	ctypes.c_int(self._screen_height), ctypes.c_int(self._screen_width),
            ctypes.c_int(self._num_rows), ctypes.c_int(self._num_cols),
            ctypes.c_int(self._num_colors))
    
    # C++ implementation that returns the size of the action set
    def getSizeActionSet(self):
        return BPROS.bpros_features.getNumberOfFeatures(self._num_rows, \
            self._num_cols, self._num_colors)