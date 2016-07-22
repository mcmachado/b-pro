/* Library implementing B-PROST features. These feature sets were originally
   introduced in the paper below. 

   Yitao Liang, Marlos C. Machado, Erik Talvitie, Michael H. Bowling:
   State of the Art Control of Atari Games Using Shallow Reinforcement Learning. 
   AAMAS 2016: 485-493

   This is not the code originally used to run those experiments. This is supposed to
   be a version that is detached from learning algorithms so people can plug and play.
   Although this file can be included and used in a C/C++ implementation (with minor
   changes), it was intended to be the basis of the Python interface.

   Author: Marlos C. Machado (part of the original code was written by Yitao Liang).
*/

#include <set>
#include <tuple>
#include <vector>

using namespace std;

/* These are just to make the code less verbose. */

typedef unsigned char u_char;
typedef vector<tuple<int,int> >::iterator t_iter;

/* Declarations required prior to its use. */

int getBasicFeaturesIndices(const u_char *screen, int screenHeight, int screenWidth,
    int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors, 
    int numRows, int numColumns, int numColors, vector<int> *features);


void addRelativeFeaturesIndices(const u_char *screen, int featureIndex,
    vector<vector<tuple<int, int> > > &whichColors, int numRows, int numColumns,
    int numColors, vector<vector<bool> > &pairwiseExistence,
    vector<tuple<int,int> > &pairwiseChanged, vector<int> *features);

void addTimeOffsetsIndices(vector<vector<tuple<int,int> > > &whichColors,
    vector<vector<tuple<int,int> > > &previousColors, int numRows, int numColumns,
    int numColors, vector<vector<bool> > &pairwiseExistence,
    vector<tuple<int,int> > &pairwiseChanged, vector<int> *features);

void resetPairwiseExistence(vector<vector<bool> >& pairwiseExistence,
    vector<tuple<int,int> >& pairwiseChanged);

extern "C" {
    vector<int>* new_vector(){
        return new vector<int>;
    }
    void delete_vector(vector<int>* v){
        delete v;
    }
    void clear_vector(vector<int>* v){
        v->clear();
    }
    int vector_size(vector<int>* v){
        return v->size();
    }
    int vector_get(vector<int>* v, int i){
        return v->at(i);
    }
}

/* This function returns the maximum number of features that can be generated given
   the number of tiles (numRows and numColumns) and the number of colors (numColors)
   to be used by this representation. This is useful knowledge so we can allocate
   vectors with the appropriate size (although they should be very sparse.
*/
extern "C" int getNumberOfFeatures(int numRows, int numColumns, int numColors){

    int numBasicFeatures = numColumns * numRows * numColors;
    int numRelativeFeatures = (2 * numColumns - 1) * (2 * numRows - 1) 
								* (1 + numColors) * numColors/2;
    int numTimeDimensionalOffsets = numColors * numColors *(2 * numColumns - 1) * (2 * numRows - 1);
    return numBasicFeatures + numRelativeFeatures +numTimeDimensionalOffsets + 1;
}

extern "C" void getBROSTFeatures(vector<int>* features, const u_char *screen, int screenHeight,
    int screenWidth, int numRows, int numColumns, int numColors){

    int blockWidth  = screenWidth  / numColumns;
    int blockHeight = screenHeight / numRows;

    vector<vector<bool> > pairwiseExistence;
    vector<tuple<int,int> > pairwiseChanged;

    vector<vector<tuple<int,int> > > previousColors;
    vector<vector<tuple<int,int> > > whichColors(numColors);

    pairwiseChanged.clear();
    pairwiseExistence.resize(2*numRows-1);
    for (int i=0; i < 2 * numRows - 1; i++){
        pairwiseExistence[i].resize(2 * numColumns - 1);
        for (int j = 0; j < 2 * numColumns - 1; j++){
            pairwiseExistence[i][j] = true;
        }
    }

    //We first get the Basic features, keeping track of the next featureIndex vector
    int featureIndex = getBasicFeaturesIndices(screen, screenHeight, screenWidth,
        blockWidth, blockHeight, whichColors, numRows, numColumns, numColors, features);
    addRelativeFeaturesIndices(screen, featureIndex, whichColors, numRows,
      numColumns, numColors, pairwiseExistence, pairwiseChanged, features);
    if (previousColors.size() == numColors){
        addTimeOffsetsIndices(whichColors, previousColors, numRows, numColumns,
          numColors, pairwiseExistence, pairwiseChanged, features);
    }
    previousColors = whichColors;

    //Bias feature
    features->push_back(getNumberOfFeatures(numRows, numColumns, numColors) - 1);
}

/* Because this is used by the Python interface, in the ALE one receives a vector
   concatenating all matrix rows. Because of that, this method is required to make
   sure we can still access the matrix given x, y coordinates.
*/
u_char getPixel(int i, int j, const u_char *screen, int screenHeight, int screenWidth){

    return screen[i * screenWidth + j];
}


int getBasicFeaturesIndices(const u_char *screen, int screenHeight, int screenWidth,
    int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors, 
    int numRows, int numColumns, int numColors, vector<int> *features){

    int featureIndex = 0;
    // For each pixel block
    for (int by = 0; by < numRows; by++) {
        for (int bx = 0; bx < numColumns; bx++) {
            //vector<boost::tuple<int, int, int> > posAndColor;
      
            int xo = bx * blockWidth;
            int yo = by * blockHeight;
            vector<bool> hasColor(numColors, false);
      
            // Determine which colors are present
            for(int x = xo; x < xo + blockWidth; x++){
                for(int y = yo; y < yo + blockHeight; y++){
                    unsigned char pixel = getPixel(y, x, screen, screenHeight, screenWidth);
                    hasColor[pixel] = true;
                }
            }

            for(int c = 0; c < numColors; c++){
                if(hasColor[c]){
                    tuple<int,int> pos (by,bx);
                    whichColors[c].push_back(pos);
                    features->push_back(featureIndex);
                }
                featureIndex++;
            }
        }
    }
    return featureIndex;
}

void addRelativeFeaturesIndices(const u_char *screen, int featureIndex,
    vector<vector<tuple<int, int> > > &whichColors, int numRows,
    int numColumns, int numColors, vector<vector<bool> > &pairwiseExistence,
    vector<tuple<int,int> > &pairwiseChanged, vector<int> *features){

    int numRowOffsets = 2 * numRows - 1;
    int numColumnOffsets = 2 * numColumns - 1;
    int numOffsets = numRowOffsets * numColumnOffsets;
    int numColorPairs = (1 + numColors) * numColors/2;
    int numBasicFeatures = numColumns * numRows * numColors;
    
    for(int c1 = 0; c1 < numColors; c1++){
        for(int k = 0; k < whichColors[c1].size(); k++){
            for(int h = 0; h < whichColors[c1].size(); h++){
                int rowDelta = get<0>(whichColors[c1][k]) - get<0>(whichColors[c1][h]);
                int columnDelta = get<1>(whichColors[c1][k]) - get<1>(whichColors[c1][h]);
                bool newBproFeature = false;
                if(rowDelta > 0){
                    newBproFeature = true;
                } else if(rowDelta == 0 && columnDelta >= 0){
                    newBproFeature = true;
                }
                rowDelta += numRows - 1;
                columnDelta += numColumns - 1;
                tuple<int,int> pos(rowDelta, columnDelta);
                pairwiseChanged.push_back(pos);
                if(newBproFeature && pairwiseExistence[rowDelta][columnDelta]){
                    pairwiseExistence[rowDelta][columnDelta] = false;
                    features->push_back(numBasicFeatures 
                      + (numColors+numColors - c1 + 1) * c1/2 * numRowOffsets * numColumnOffsets 
                      + rowDelta * numColumnOffsets + columnDelta);
                }
            }
        }
        resetPairwiseExistence(pairwiseExistence, pairwiseChanged);
        
        for(int c2 = c1 + 1; c2 < numColors; c2++){
            if(whichColors[c1].size() > 0 && whichColors[c2].size() > 0){
                for(int it1 = 0; it1 < whichColors[c1].size(); it1++){
                    for(int it2 = 0; it2 < whichColors[c2].size(); it2++){
                        int rowDelta = get<0>(whichColors[c1][it1])
                            - get<0>(whichColors[c2][it2]) + numRows - 1;
                        int columnDelta = get<1>(whichColors[c1][it1])
                            - get<1>(whichColors[c2][it2]) + numColumns - 1;
                        tuple<int,int> pos(rowDelta,columnDelta);
                        pairwiseChanged.push_back(pos);
                        long long index = numBasicFeatures + 
                            (long long)((numColors + numColors - c1 + 1) * c1/2 * numRowOffsets
                            * numColumnOffsets) + (long long)((c2 - c1) * numRowOffsets
                            * numColumnOffsets) + (long long)rowDelta * numColumnOffsets
                            + (long long)columnDelta;
                        if(pairwiseExistence[rowDelta][columnDelta]){
                            pairwiseExistence[rowDelta][columnDelta] = false;
                            features->push_back(index);
                        }
                    }
                }
            }
            resetPairwiseExistence(pairwiseExistence, pairwiseChanged);
        }
    }
}

void addTimeOffsetsIndices(vector<vector<tuple<int,int> > > &whichColors,
    vector<vector<tuple<int,int> > > &previousColors, int numRows, int numColumns,
    int numColors, vector<vector<bool> > &pairwiseExistence,
    vector<tuple<int,int> > &pairwiseChanged, vector<int> *features){

    int numRowOffsets = 2 * numRows - 1;
    int numColumnOffsets = 2 * numColumns - 1;
    int numBasicFeatures = numColumns * numRows * numColors;
    int numRelativeFeatures = (2 * numColumns - 1) * (2 * numRows - 1) 
                * (1 + numColors) * numColors/2;

    for(int c1 = 0; c1 < numColors; c1++){
        for(int c2 = 0; c2 < numColors; c2++){
            if(previousColors[c1].size() > 0 && whichColors[c2].size() > 0){
                for(t_iter it1 = previousColors[c1].begin(); it1 != previousColors[c1].end(); it1++){
                    for(t_iter it2 = whichColors[c2].begin(); it2 != whichColors[c2].end(); it2++){
                        int rowDelta = get<0>(*it1) - get<0>(*it2) + numRows - 1;
                        int columnDelta = get<1>(*it1) - get<1>(*it2)+numColumns - 1;
                        if(pairwiseExistence[rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta,columnDelta);
                            pairwiseChanged.push_back(pos);
                            pairwiseExistence[rowDelta][columnDelta] = false;
                            features->push_back(numBasicFeatures + numRelativeFeatures 
                                + c1 * numColors * numRowOffsets * numColumnOffsets
                                + c2 * numRowOffsets * numColumnOffsets
                                + rowDelta * numColumnOffsets + columnDelta);
                        }
                    }
                }
            }
            resetPairwiseExistence(pairwiseExistence, pairwiseChanged);
        }
    }
}

void resetPairwiseExistence(vector<vector<bool> >& pairwiseExistence,
    vector<tuple<int,int> >& pairwiseChanged){
    
    for(t_iter it = pairwiseChanged.begin(); it != pairwiseChanged.end(); it++){
        pairwiseExistence[get<0>(*it)][get<1>(*it)] = true;
    }
    pairwiseChanged.clear();
}
