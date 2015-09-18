/****************************************************************************************
** Implementation of a variation of BASS Features, which has features to encode the 
**  relative position between tiles.
**
** REMARKS: - This implementation is basically Erik Talvitie's implementation, presented
**            in the AAAI'15 LGCVG Workshop.
**
** Author: Marlos C. Machado
***************************************************************************************/

#ifndef BPRO_FEATURES_H
#define BPRO_FEATURES_H
#include "BPROFeatures.hpp"
#endif
#ifndef BASIC_FEATURES_H
#define BASIC_FEATURES_H
#include "BasicFeatures.hpp"
#endif

#include <set>
#include <assert.h>
using namespace std;
//#include <tuple>
//#include <boost/tuple/tuple.hpp> //TODO: I have to remove this to not have to depend on boost

BPROFeatures::BPROFeatures(Parameters *param){
    this->param = param;
    numColumns  = param->getNumColumns();
	numRows     = param->getNumRows();
	numColors   = param->getNumColors();

	if(this->param->getSubtractBackground()){
        this->background = new Background(param);
    }

	//To get the total number of features:
	//TODO: Fix this!
    numBasicFeatures = this->param->getNumColumns() * this->param->getNumRows() * this->param->getNumColors();
	numRelativeFeatures = (2 * this->param->getNumColumns() - 1) * (2 * this->param->getNumRows() - 1) 
							* (1+this->param->getNumColors()) * this->param->getNumColors()/2;
    changed.clear();
    bproExistence.resize(2*numRows-1);
    for (int i=0;i<2*numRows-1;i++){
        bproExistence[i].resize(2*numColumns-1);
        for (int j=0;j<2*numColumns-1;j++){
            bproExistence[i][j]=true;
        }
    }
}

BPROFeatures::~BPROFeatures(){}

int BPROFeatures::getBasicFeaturesIndices(const ALEScreen &screen, int blockWidth, int blockHeight, 
                                          vector<vector<tuple<int,int> > > &whichColors, vector<int>& features){
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

void BPROFeatures::addRelativeFeaturesIndices(const ALEScreen &screen, int featureIndex,
	vector<vector<tuple<int,int> > > &whichColors, vector<int>& features){

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
                if (newBproFeature && bproExistence[rowDelta][columnDelta]){
                    tuple<int,int> pos (rowDelta,columnDelta);
                    changed.push_back(pos);
                    bproExistence[rowDelta][columnDelta]=false;
                    features.push_back(numBasicFeatures+(numColors+numColors-c1+1)*c1/2*numRowOffsets*numColumnOffsets+rowDelta*numColumnOffsets+columnDelta);
                    
                }
            }
        }
        resetBproExistence(bproExistence,changed);

        for (int c2=c1+1;c2<numColors;c2++){
            if (whichColors[c1].size()>0 && whichColors[c2].size()>0){
                for (vector<tuple<int,int> >::iterator it1=whichColors[c1].begin();it1!=whichColors[c1].end();it1++){
                    for (vector<tuple<int,int> >::iterator it2=whichColors[c2].begin();it2!=whichColors[c2].end();it2++){
                        int rowDelta = get<0>(*it1)-get<0>(*it2)+numRows-1;
                        int columnDelta = get<1>(*it1)-get<1>(*it2)+numColumns-1;
                        if (bproExistence[rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta,columnDelta);
                            changed.push_back(pos);
                            bproExistence[rowDelta][columnDelta]=false;
                            features.push_back(numBasicFeatures+(numColors+numColors-c1+1)*c1/2*numRowOffsets*numColumnOffsets+(c2-c1)*numRowOffsets*numColumnOffsets+rowDelta*numColumnOffsets+columnDelta);
                        }
                    }
                }
            }
            resetBproExistence(bproExistence,changed);
        }
    }    
}

void BPROFeatures::getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<int>& features){
	int screenWidth = screen.width();
	int screenHeight = screen.height();
	int blockWidth = screenWidth / numColumns;
	int blockHeight = screenHeight / numRows;

	assert(features.size() == 0); //If the vector is not empty this can be a mess
    vector<vector<tuple<int,int> > > whichColors(numColors);

    //Before generating features we must check whether we can subtract the background:
    if(this->param->getSubtractBackground()){
        unsigned int sizeBackground = this->background->getWidth() * this->background->getHeight();
        assert(sizeBackground == screen.width()*screen.height());
    }

    //We first get the Basic features, keeping track of the next featureIndex vector:
    //We don't just use the Basic implementation because we need the whichColors information
	int featureIndex = getBasicFeaturesIndices(screen, blockWidth, blockHeight, whichColors, features);
	addRelativeFeaturesIndices(screen, featureIndex, whichColors, features);

	//Bias
	features.push_back(numBasicFeatures+numRelativeFeatures);
}

int BPROFeatures::getNumberOfFeatures(){
    return numBasicFeatures + numRelativeFeatures + 1;
}

void BPROFeatures::resetBproExistence(vector<vector<bool> >& bproExistence, vector<tuple<int,int> >& changed){
    for (vector<tuple<int,int> >::iterator it = changed.begin();it!=changed.end();it++){
        bproExistence[get<0>(*it)][get<1>(*it)]=true;
    }
    changed.clear();
}