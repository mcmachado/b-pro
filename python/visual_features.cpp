#include <set>
#include <tuple>
#include <vector>

using namespace std;

int getBasicFeaturesIndices(const int **screen, int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors, int numRows, int numColumns, int numColors, vector<int>& features);
void addRelativeFeaturesIndices(const int **screen, int featureIndex, vector<vector<tuple<int, int> > > &whichColors, long numRows, long numColumns, long numColors, vector<int>& features);
void resetBproExistence(vector<vector<bool> >& bproExistence, vector<tuple<int,int> >& changed);

extern "C" int getNumberOfFeatures(long numRows, long numColumns, long numColors){
	long long numBasicFeatures = numColumns * numRows * numColors;
	long long numRelativeFeatures = (2 * numColumns - 1) * (2 * numRows - 1) * (1 + numColors) * numColors/2;
    return numBasicFeatures + numRelativeFeatures + 1;
}

extern "C" void getBROSFeatures(const int **screen, long screenWidth, long screenHeight, long numRows, long numColumns, long numColors){

	vector<int> features;
	int blockWidth   = screenWidth / numColumns;
	int blockHeight  = screenHeight / numRows;

	vector<vector<tuple<int, int> > > whichColors(numColors);

	// We first get the Basic features, keeping track of the next featureIndex vector.
	// We don't just use the Basic implementation because we need the whichColors information
	int featureIndex = getBasicFeaturesIndices(screen, blockWidth, blockHeight, whichColors, numRows, numColumns, numColors, features);
	addRelativeFeaturesIndices(screen, featureIndex, whichColors, numRows, numColumns, numColors, features);

	//Bias
	features.push_back(getNumberOfFeatures(numRows, numColumns, numColors));
}

int getBasicFeaturesIndices(const int **screen, int blockWidth, int blockHeight, vector<vector<tuple<int, int> > > &whichColors, int numRows, int numColumns, int numColors, vector<int>& features){
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
					unsigned char pixel = screen[y][x];
					pixel = pixel >> 1;
					hasColor[pixel] = true;
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

void addRelativeFeaturesIndices(const int **screen, int featureIndex, vector<vector<tuple<int, int> > > &whichColors, long numRows, long numColumns, long numColors, vector<int>& features){

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
	long long numBasicFeatures = numColumns * numRows * numColors;
    
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
                    features.push_back(numBasicFeatures + (numColors + numColors - c1 + 1) * c1/2 * numRowOffsets * numColumnOffsets + rowDelta * numColumnOffsets + columnDelta);
                }
            }
        }
        resetBproExistence(bproExistence,changed);

        for(int c2 = c1 + 1; c2 < numColors; c2++){
            if(whichColors[c1].size() > 0 && whichColors[c2].size() > 0){
                for(vector<tuple<int,int> >::iterator it1 = whichColors[c1].begin(); it1 != whichColors[c1].end(); it1++){
                    for(vector<tuple<int,int> >::iterator it2 = whichColors[c2].begin(); it2 != whichColors[c2].end(); it2++){
                        int rowDelta = get<0>(*it1) - get<0>(*it2) + numRows - 1;
                        int columnDelta = get<1>(*it1) - get<1>(*it2) + numColumns - 1;
                        if(bproExistence[rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta, columnDelta);
                            changed.push_back(pos);
                            bproExistence[rowDelta][columnDelta] = false;
                            features.push_back(numBasicFeatures + (numColors + numColors - c1 + 1) * c1/2 * numRowOffsets * numColumnOffsets + (c2 - c1) * numRowOffsets * numColumnOffsets + rowDelta * numColumnOffsets + columnDelta);
                        }
                    }
                }
            }
            resetBproExistence(bproExistence,changed);
        }
    }
}


void resetBproExistence(vector<vector<bool> >& bproExistence, vector<tuple<int,int> >& changed){
    for (vector<tuple<int,int> >::iterator it = changed.begin(); it!=changed.end(); it++){
        bproExistence[get<0>(*it)][get<1>(*it)] = true;
    }
    changed.clear();
}