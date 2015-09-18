/****************************************************************************************
** Superclass of all other classes that define Features. It is required from classes that
** inherit from Features to implement the virtual methods getCompleteFeatureVector and
** getNumberOfFeatures, which are operations specific to the kind of selected features.
** It already implements getActiveFeaturesIndices, which allow optimizations for the 
** learning algorithms.
**
** REMARKS: - All methods' high-level comments are in the .hpp file.
** 
** TODO: Right now this class only deals with binary features, it may be necessary to
**       further extend such approach.
**
** Author: Marlos C. Machado
***************************************************************************************/

#include "Features.hpp"

void Features::getCompleteFeatureVector(const ALEScreen &screen, const ALERAM &ram, vector<bool>& features){	
	assert(features.size() == 0); //If the vector is not empty this can be a mess
	//Get vector with active features:
	vector<int> temp;
	vector<int>& t = temp;
	this->getActiveFeaturesIndices(screen, ram, t);
	//Iterate over vector with all features storing the non-zero indices in the new vector:
	features = vector<bool>(this->getNumberOfFeatures(), 0);
	for(unsigned int i = 0; i < t.size(); i++){
		features[t[i]] = 1;
	}
}

Features::~Features(){}
