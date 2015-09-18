/****************************************************************************************
** Mathematical tools that may be useful across several applications, even outside ALE
** environment. This class is meant to be static since ideally this should only implement
** functions. Adding non-static objects should be further discussed before implemented.
** 
** Author: Marlos C. Machado
***************************************************************************************/

#ifndef MATHEMATICS_H
#define MATHEMATICS_H
#include "Mathematics.hpp"
#endif
#include <assert.h>
#include <cstdlib>
#include <random>

int Mathematics::argmax(std::vector<float> array){
	assert(array.size() > 0);
	//Discover max value of the array:
	double max = array[0];
	for (unsigned int i = 0; i < array.size(); i++){
		if(max < array[i]){
			max = array[i];
		}
	}
	//We need to break ties, thus we save all  
	//indices that hold the same max value:
	std::vector<int> indices;
	for(unsigned int i = 0; i < array.size(); i++){
		if(fabs(array[i] - max) < 1e-10){
			indices.push_back(i);
		}
	}
	assert(indices.size() > 0);
	//Now we randomly pick one of the best
	return indices[rand()%indices.size()];
}

int Mathematics::argmax(std::vector<float>array,std::mt19937& randAgent){
    assert(array.size() > 0);
    //Discover max value of the array:
    double max = array[0];
    for (unsigned int i = 0; i < array.size(); i++){
        if(max < array[i]){
            max = array[i];
        }
    }
    //We need to break ties, thus we save all
    //indices that hold the same max value:
    std::vector<int> indices;
    for(unsigned int i = 0; i < array.size(); i++){
        if(fabs(array[i] - max) < 1e-6){
            indices.push_back(i);
        }
    }
    assert(indices.size() > 0);
    //Now we randomly pick one of the best
    return indices[randAgent()%indices.size()];
}

