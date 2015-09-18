/****************************************************************************************
** Superclass of all other classes that define Features. It is required from classes that
** inherit from Features to implement the virtual methods getActiveFeaturesIndices and
** getNumberOfFeatures, which are operations specific to the kind of selected features.
** It already implements getCompleteFeatureVector, which can be used if someone needs the
** complete feature vector. This is not an efficient approach.
** 
** TODO: Right now this class only deals with binary features, it may be necessary to
**       further extend such approach.
**
** Author: Marlos C. Machado
***************************************************************************************/

#ifndef ALE_INTERFACE_H
#define ALE_INTERFACE_H
#include <ale_interface.hpp>
#endif

using namespace std;

class Features{
	private:
		
	public:
		/**
 		* This method was created to allow optimizations in codes that use such features.
 		* Since several feature definitions generate sparse feature vectors it is much
 		* better to just iterate over the active features. This method allows it by
 		* providing a vector with the indices of active features. Recall we assume the
 		* features to be binary. Each implementation of a feature needs to instantiate this
 		* method.
 		*
 		* REMARKS: - It is necessary to provide both the screen and the ram as one may
 		* want to use both data to generate features.
 		*          - To avoid return huge vectors, this method is void and the appropriate
 		* vector is returned trough a parameter passed by reference
 		*
 		* TODO: If one intends to use non-binary features this class is not suitable.
 		*
 		* @param ALEScreen &screen is the current game screen that one may use to extract features.
 		* @param ALERAM &ram is the current game RAM that one may use to extract features.
 		* @param vector<int>& features an empy vector that will be filled with the requested information,
 		*        therefore it must be passed by reference.
 		* 
 		* @return nothing since one will receive the requested data by the last parameter, by reference.
 		*/
		virtual void getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<long long>& features) = 0;
		/**
 		* It 'returns' a binary vector containing 1's where the feature is active. Ideally this
 		* method will never be used as iterating over all features is far less efficient than
 		* iterating over the set of active features.
 		*
 		* REMARKS: - It is necessary to provide both the screen and the ram as one may
 		* want to use both data to generate features.
 		*          - To avoid return huge vectors, this method is void and the appropriate
 		* vector is returned trough a parameter passed by reference
 		*
 		* TODO: If one intends to use non-binary features this class is not suitable.
 		*
 		* @param ALEScreen &screen is the current game screen that one may use to extract features.
 		* @param ALERAM &ram is the current game RAM that one may use to extract features.
 		* @param vector<bool>& features an empy vector that will be filled with the requested information,
 		*        therefore it must be passed by reference. Its i-th position is TRUE if the i-th feature is active.
 		* @return nothing since one will receive the requested data by the last parameter, by reference.
 		*/
		void getCompleteFeatureVector(const ALEScreen &screen, const ALERAM &ram, vector<bool>& features);
		/**
 		* This pure virtual method must be implemented by every class inhereting from this one.
 		* It returns the number of features existent in the defined representation. It is the total size,
 		* not necessarily the number of features active or anything similar. Notice it does not even
 		* has access to the feature vector.
 		*
 		* @param none
 		* @return integer representing the number of features of a given representation.
 		*/
		virtual long long getNumberOfFeatures() = 0;
		/**
		* Destructor, not necessary in this class.
		*/
		virtual ~Features();
        virtual void clearCash() = 0 ;
};
