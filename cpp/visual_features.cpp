/* Library implementing B-PROS, B-PROST and Blob-PROST features. These feature sets
   were originally introduced in the paper below. 

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
#include <iostream>

#include "visual_features.hpp"

using namespace std;

/* Declarations required prior to its use. */

int getBasicFeaturesIndices(const u_char *screen, int screenHeight, int screenWidth,
    int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors, 
    int numRows, int numColumns, int numColors, vector<int> *features);


void addRelativeFeaturesIndices(const u_char *screen, int featureIndex,
    vector<vector<tuple<int, int> > > &whichColors, int numRows,
    int numColumns, int numColors, vector<int> *features);

void resetBproExistence(vector<vector<bool> >& bproExistence, vector<tuple<int,int> >& changed);

/* These are just to make the code less verbose. */

typedef unsigned char u_char;
typedef vector<tuple<int,int> >::iterator t_iter;

/* This function returns the maximum number of features that can be generated given
   the number of tiles (numRows and numColumns) and the number of colors (numColors)
   to be used by this representation. This is useful knowledge so we can allocate
   vectors with the appropriate size (although they should be very sparse.
*/
int getNumberOfFeatures(int numRows, int numColumns, int numColors){
    
    long long numBasicFeatures = numColumns * numRows * numColors;
    long long numRelativeFeatures = 
        (2 * numColumns - 1) * (2 * numRows - 1) * (1 + numColors) * numColors/2;
    return numBasicFeatures + numRelativeFeatures + 1;
}


/* This function returns a vector containing the indices that are not zero in the
   representation. Because the representation is binary, it is much more efficient
   to generate the indices than to generate the whole (huge) vector with a bunch of
   zeros. B-PROS features are described in the reference aforementioned.
*/
void getBROSFeatures(vector<int>* features, const u_char *screen, int screenHeight,
    int screenWidth, int numRows, int numColumns, int numColors){

	//vector<int> features;
	int blockWidth   = screenWidth / numColumns;
	int blockHeight  = screenHeight / numRows;

	vector<vector<tuple<int, int> > > whichColors(numColors);

	// We first get the Basic features, keeping track of the next featureIndex vector.
	int featureIndex = getBasicFeaturesIndices(screen, screenHeight, screenWidth,
        blockWidth, blockHeight, whichColors, numRows, numColumns, numColors, features);

    // We now add the PROS features, the pairwise combination of pixels.
    addRelativeFeaturesIndices(screen, featureIndex, whichColors,
        numRows, numColumns, numColors, features);

	// Bias
	features->push_back(getNumberOfFeatures(numRows, numColumns, numColors));
}

/* Because this is used by the Python interface, in the ALE one receives a vector
   concatenating all matrix rows. Because of that, this method is required to make
   sure we can still access the matrix given x, y coordinates.
*/
u_char getPixel(int i, int j, const u_char *screen, int screenHeight, int screenWidth){

    return screen[i * screenWidth + j];
}

/* Implementation of Basic features, originally proposed in the paper below.

   Marc G. Bellemare, Yavar Naddaf, Joel Veness, Michael Bowling:
   The Arcade Learning Environment: An Evaluation Platform for General Agents. 
   J. Artif. Intell. Res. (JAIR) 47: 253-279 (2013)
*/
int getBasicFeaturesIndices(const u_char *screen, int screenHeight, int screenWidth,
    int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors,
    int numRows, int numColumns, int numColors, vector<int> *features){

    int featureIndex = 0;
	// For each pixel block
	for (int by = 0; by < numRows; by++) {
		for (int bx = 0; bx < numColumns; bx++) {
			int xo = bx * blockWidth;
			int yo = by * blockHeight;
			vector<bool> hasColor(numColors, false);
			
			// Determine which colors are present
			for (int x = xo; x < xo + blockWidth; x++){
				for (int y = yo; y < yo + blockHeight; y++){
					u_char pixel = getPixel(y, x, screen, screenHeight, screenWidth);
					pixel = pixel >> 1;
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

/* This function is the one that really implement the 'PROS' part of B-PROS.
   It adds all the pairwise combinations of relative positions on the screen.
   See reference available in the header for further details.
*/
void addRelativeFeaturesIndices(const u_char *screen, int featureIndex,
    vector<vector<tuple<int, int> > > &whichColors, int numRows,
    int numColumns, int numColors, vector<int> *features){

	vector<tuple<int,int> > changed;
	vector<vector<bool> > bproExistence;
    bproExistence.resize(2 * numRows - 1);

    for (int i = 0; i < 2 * numRows - 1; i++){
        bproExistence[i].resize(2 * numColumns - 1);
        for (int j = 0; j < 2 * numColumns-1; j++){
            bproExistence[i][j] = true;
        }
    }

	int numRowOffsets    = 2 * numRows - 1;
	int numColumnOffsets = 2 * numColumns - 1;
	int numOffsets       = numRowOffsets * numColumnOffsets;
	int numColorPairs    = (1 + numColors) * numColors/2;
	int numBasicFeatures = numColumns * numRows * numColors;
    
    for(int c1 = 0;c1 < numColors; c1++){
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
                if(newBproFeature && bproExistence[rowDelta][columnDelta]){
                    tuple<int,int> pos(rowDelta,columnDelta);
                    changed.push_back(pos);
                    bproExistence[rowDelta][columnDelta] = false;
                    features->push_back(numBasicFeatures + (numColors + numColors - c1 + 1) 
                        * c1/2 * numRowOffsets * numColumnOffsets 
                        + rowDelta * numColumnOffsets + columnDelta);
                }
            }
        }
        resetBproExistence(bproExistence,changed);

        for(int c2 = c1 + 1; c2 < numColors; c2++){
            if(whichColors[c1].size() > 0 && whichColors[c2].size() > 0){
                for(t_iter it1 = whichColors[c1].begin(); it1 != whichColors[c1].end(); it1++){
                    for(t_iter it2 = whichColors[c2].begin(); it2 != whichColors[c2].end(); it2++){
                        int rowDelta = get<0>(*it1) - get<0>(*it2) + numRows - 1;
                        int columnDelta = get<1>(*it1) - get<1>(*it2) + numColumns - 1;
                        if(bproExistence[rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta, columnDelta);
                            changed.push_back(pos);
                            bproExistence[rowDelta][columnDelta] = false;
                            features->push_back(numBasicFeatures + (numColors + numColors - c1 + 1)
                                * c1/2 * numRowOffsets * numColumnOffsets
                                + (c2 - c1) * numRowOffsets * numColumnOffsets
                                + rowDelta * numColumnOffsets + columnDelta);
                        }
                    }
                }
            }
            resetBproExistence(bproExistence,changed);
        }
    }
}

/* Just a simple method that reset the bproExistence vector. */
void resetBproExistence(vector<vector<bool> >& bproExistence, vector<tuple<int,int> >& changed){

    for (t_iter it = changed.begin(); it!=changed.end(); it++){
        bproExistence[get<0>(*it)][get<1>(*it)] = true;
    }
    changed.clear();
}
