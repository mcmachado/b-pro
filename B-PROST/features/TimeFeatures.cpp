/****************************************************************************************
** Implementation of a variation of BASS Features, which has features to encode the 
**  relative position between tiles.
**
** REMARKS: - This implementation is basically Erik Talvitie's implementation, presented
**            in the AAAI'15 LGCVG Workshop.
**
** Author: Marlos C. Machado
***************************************************************************************/

#ifndef TIME_FEATURES_H
#define TIME_FEATURES_H
#include "TimeFeatures.hpp"
#endif

#include <set>
#include <assert.h>
using namespace std;

TimeFeatures::TimeFeatures(Parameters *param){
    this->param = param;
    numColumns  = param->getNumColumns();
	numRows     = param->getNumRows();
	numColors   = param->getNumColors();

	if(this->param->getSubtractBackground()){
        this->background = new Background(param);
    }

    numBasicFeatures = this->param->getNumColumns() * this->param->getNumRows() * this->param->getNumColors();
	numRelativeFeatures = (2 * this->param->getNumColumns() - 1) * (2 * this->param->getNumRows() - 1) 
							* (1+this->param->getNumColors()) * this->param->getNumColors()/2;
    numTimeDimensionalOffsets = this->param->getNumColors() * this->param->getNumColors() *(2 * this->param->getNumColumns() - 1) * (2 * this->param->getNumRows() - 1) ;
    
    pairwiseChanged.clear();
    pairwiseExistence.resize(2*numRows-1);
    for (int i=0;i<2*numRows-1;i++){
        pairwiseExistence[i].resize(2*numColumns-1);
        for (int j=0;j<2*numColumns-1;j++){
            pairwiseExistence[i][j]=true;
        }
    }
}

TimeFeatures::~TimeFeatures(){}

int TimeFeatures::getBasicFeaturesIndices(const ALEScreen &screen, int blockWidth, int blockHeight,
                                          vector<vector<tuple<int,int> > > &whichColors, vector<long long>& features){
	int featureIndex = 0;
	// For each pixel block
	for (int by = 0; by < numRows; by++) {
		for (int bx = 0; bx < numColumns; bx++) {
			//vector<boost::tuple<int, int, int> > posAndColor;
			
			int xo = bx * blockWidth;
			int yo = by * blockHeight;
			vector<bool> hasColor(numColors, false);
			
			// Determine which colors are present
			for (int x = xo; x < xo + blockWidth; x++){
				for (int y = yo; y < yo + blockHeight; y++){
					unsigned char pixel = screen.get(y,x);

					if(!this->param->getSubtractBackground() || (this->background->getPixel(y, x) != pixel)){
						if(numColors == 8){ //SECAM, considering only 8 colors
							pixel = (pixel & 0xF) >> 1;
						}
						else if(numColors == 128){ //NTSC, considering 128 colors
							pixel = pixel >> 1;
						}
		  				
		  				hasColor[pixel] = true;
						//posAndColor.push_back(boost::make_tuple(x, y, pixel));
					}
				}
			}

			for(int c = 0; c < numColors; c++){
				if(hasColor[c]){
                    tuple<int,int> pos (by,bx);
					whichColors[c].push_back(pos);
					features.push_back(featureIndex);
				}
				featureIndex++;
			}
		}
	}
	return featureIndex;
}

void TimeFeatures::addRelativeFeaturesIndices(const ALEScreen &screen, long long featureIndex,
	vector<vector<tuple<int,int> > > &whichColors, vector<long long>& features){

	int numRowOffsets = 2*numRows - 1;
	int numColumnOffsets = 2*numColumns - 1;
	int numOffsets = numRowOffsets*numColumnOffsets;
	int numColorPairs = (1+numColors)*numColors/2;
    
    for (int c1=0;c1<numColors;c1++){
        for (int k=0;k<whichColors[c1].size();k++){
            for (int h=0;h<whichColors[c1].size();h++){
                int rowDelta = get<0>(whichColors[c1][k])-get<0>(whichColors[c1][h]);
                int columnDelta = get<1>(whichColors[c1][k])-get<1>(whichColors[c1][h]);
                bool newBproFeature = false;
                if (rowDelta>0){
                    newBproFeature = true;
                }else if (rowDelta==0 && columnDelta >=0){
                    newBproFeature = true;
                }
                rowDelta+=numRows-1;
                columnDelta+=numColumns-1;
                tuple<int,int> pos (rowDelta,columnDelta);
                pairwiseChanged.push_back(pos);
                if (newBproFeature && pairwiseExistence[rowDelta][columnDelta]){
                    pairwiseExistence[rowDelta][columnDelta]=false;
                    features.push_back(numBasicFeatures+(numColors+numColors-c1+1)*c1/2*numRowOffsets*numColumnOffsets+rowDelta*numColumnOffsets+columnDelta);
                }

            }
        }
        resetPairwiseExistence();
        
        for (int c2=c1+1;c2<numColors;c2++){
            if (whichColors[c1].size()>0 && whichColors[c2].size()>0){
                for (int it1=0;it1<whichColors[c1].size();it1++){
                    for (int it2=0;it2<whichColors[c2].size();it2++){
                        int rowDelta = get<0>(whichColors[c1][it1])-get<0>(whichColors[c2][it2])+numRows-1;
                        int columnDelta = get<1>(whichColors[c1][it1])-get<1>(whichColors[c2][it2])+numColumns-1;
                        tuple<int,int> pos(rowDelta,columnDelta);
                        pairwiseChanged.push_back(pos);
                        long long index=numBasicFeatures+(long long)((numColors+numColors-c1+1)*c1/2*numRowOffsets*numColumnOffsets)+(long long)((c2-c1)*numRowOffsets*numColumnOffsets)+(long long)rowDelta*numColumnOffsets+(long long)columnDelta;
                        if (pairwiseExistence[rowDelta][columnDelta]){
                            pairwiseExistence[rowDelta][columnDelta]=false;
                            features.push_back(index);
                        }
                        
                    }
                }
            }
            resetPairwiseExistence();
        }
    }    
}

void TimeFeatures::addTimeOffsetsIndices(vector<vector<tuple<int,int> > > &whichColors, vector<long long>& features){
    int numRowOffsets = 2*numRows - 1;
    int numColumnOffsets = 2*numColumns - 1;
    for (int c1=0;c1<numColors;c1++){
        for (int c2=0;c2<numColors;c2++){
            if (previousColors[c1].size()>0 && whichColors[c2].size()>0){
                for (vector<tuple<int,int> >::iterator it1=previousColors[c1].begin();it1!=previousColors[c1].end();it1++){
                    for (vector<tuple<int,int> >::iterator it2=whichColors[c2].begin();it2!=whichColors[c2].end();it2++){
                        int rowDelta = get<0>(*it1)-get<0>(*it2)+numRows-1;
                        int columnDelta = get<1>(*it1)-get<1>(*it2)+numColumns-1;
                        if (pairwiseExistence[rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta,columnDelta);
                            pairwiseChanged.push_back(pos);
                            pairwiseExistence[rowDelta][columnDelta]=false;
                            features.push_back(numBasicFeatures+numRelativeFeatures+c1*numColors*numRowOffsets*numColumnOffsets+c2*numRowOffsets*numColumnOffsets+rowDelta*numColumnOffsets+columnDelta);
                        }
                    }
                }
            }
            resetPairwiseExistence();
        }
    }
}

void TimeFeatures::getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<long long>& features){
    int screenWidth = 160;
    int screenHeight = 210;
	int blockWidth = screenWidth / numColumns;
	int blockHeight = screenHeight / numRows;

	assert(features.size() == 0); //If the vector is not empty this can be a mess
    vector<vector<tuple<int,int> > > whichColors(numColors);

    //Before generating features we must check whether we can subtract the background:
    if(this->param->getSubtractBackground()){
        unsigned int sizeBackground = this->background->getWidth() * this->background->getHeight();
        assert(sizeBackground == 160*210);
    }

    //We first get the Basic features, keeping track of the next featureIndex vector:
    //We don't just use the Basic implementation because we need the whichColors information
	int featureIndex = getBasicFeaturesIndices(screen, blockWidth, blockHeight, whichColors, features);
	addRelativeFeaturesIndices(screen, featureIndex, whichColors, features);
    if (previousColors.size()==numColors){
        addTimeOffsetsIndices(whichColors,features);
    }
    previousColors = whichColors;
    
	//Bias feature
	features.push_back(numBasicFeatures+numRelativeFeatures+numTimeDimensionalOffsets);
}

long long TimeFeatures::getNumberOfFeatures(){
    return numBasicFeatures + numRelativeFeatures +numTimeDimensionalOffsets+1;
}

void TimeFeatures::resetPairwiseExistence(){
    for (vector<tuple<int,int> >::iterator it = pairwiseChanged.begin();it!=pairwiseChanged.end();it++){
        pairwiseExistence[get<0>(*it)][get<1>(*it)]=true;
    }
    pairwiseChanged.clear();
}

void TimeFeatures::clearCash(){
    previousColors.clear();
}