/****************************************************************************************
** Mathematical tools that may be useful across several applications, even outside ALE
** environment. This class is meant to be static since ideally this should only implement
** functions. Adding non-static objects should be further discussed before implemented.
** 
** Author: Marlos C. Machado
***************************************************************************************/

#include <vector>
#include <math.h>
#include <random>

class Mathematics{
	public:
	   /**
 		* Implementation of argmax function. It breaks ties randomly.
 		*
 		* TODO: Right now it is implemented for vectors of doubles, it should
 		* be parametrized for any type. Definetely Templates should be used 
 		* here in the future.
 		*
 		* @param std::vector<double> array vector one wants the argmax
 		*
 		* @return indice of an element with highest value, ties are broke randomly.
 		*/
		static int argmax(std::vector<float> array);
    static int argmax(std::vector<float>array,std::mt19937* randAgent);
};
