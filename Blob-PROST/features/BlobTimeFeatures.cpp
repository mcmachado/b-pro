/****************************************************************************************
 ** Implementation of a variation of BASS Features, which has features to encode the
 **  relative position between tiles.
 **
 ** REMARKS: - This implementation is basically Erik Talvitie's implementation, presented
 **            in the AAAI'15 LGCVG Workshop.
 **
 ** Author: Marlos C. Machado
 ***************************************************************************************/

#ifndef Blob_TIME_FEATURES_H
#define Blob_TIME_FEATURES_H
#include "BlobTimeFeatures.hpp"
#endif

#include <set>
#include <assert.h>
#include <algorithm>
#include <math.h>
#include <unordered_set>

using namespace std;
//using google::dense_hash_map;


BlobTimeFeatures::BlobTimeFeatures(Parameters *param){
    this->param = param;
    //numColumns  = param->getNumColumns();
    //numRows     = param->getNumRows();
    numColors   = param->getNumColors();
    colorMultiplier =(int) log2(256 / numColors);
    
    if(this->param->getSubtractBackground()){
        this->background = new Background(param);
    }

    //To get the total number of features:
    numBasicFeatures = 0;
    numRelativeFeatures = 0;
    numTimeDimensionalOffsets = 0;
    //numThreePointOffsets = 0;
    
    auto resolutionsToRead = param->getResolutions();
    for (int i=0;i<resolutionsToRead.size()/2;++i){
        resolutions.push_back(make_tuple(resolutionsToRead[2*i],resolutionsToRead[2*i+1]));
    }
    numResolutions = resolutions.size();
    
    for (int index=0;index<numResolutions;index++){
        numBlocks.push_back(make_tuple(210/get<0>(resolutions[index]),160/get<1>(resolutions[index])));
        numOffsets.push_back(make_tuple(2 * get<0>(numBlocks[index])-1, 2*get<1>(numBlocks[index])-1));
        numBasicFeatures+= numColors*get<0>(numBlocks[index])*get<1>(numBlocks[index]);
        numRelativeFeatures+= get<0>(numOffsets[index]) * get<1>(numOffsets[index])* (1+numColors) * numColors/2;
        numTimeDimensionalOffsets+= get<0>(numOffsets[index]) * get<1>(numOffsets[index]) * numColors * numColors;
        //numThreePointOffsets+= get<0>(numOffsets[index]) * get<1>(numOffsets[index])* (1+numColors) * numColors/2 * numColors * get<0>(numOffsets[index])*get<1>(numOffsets[index]);
    }
    //get different base for calculation
    baseBasic.push_back(0);
    baseBpro.push_back(numBasicFeatures);
    baseTime.push_back(numBasicFeatures+numRelativeFeatures);
    baseThreePoint.push_back(numBasicFeatures+numRelativeFeatures+numTimeDimensionalOffsets);
    for (int index=0;index<numResolutions-1;++index){
        baseBasic.push_back(baseBasic.back()+numColors*get<0>(numBlocks[index])*get<1>(numBlocks[index]));
        baseBpro.push_back(baseBpro.back()+get<0>(numOffsets[index]) * get<1>(numOffsets[index])* (1+numColors) * numColors/2);
        baseTime.push_back(baseTime.back()+ get<0>(numOffsets[index]) * get<1>(numOffsets[index]) * numColors * numColors);
        //baseThreePoint.push_back(baseThreePoint.back()+get<0>(numOffsets[index]) * get<1>(numOffsets[index])* (1+numColors) * numColors/2 * numColors* get<0>(numOffsets[index])*get<1>(numOffsets[index]));
    }
    
    //set up table to prevent repetitive features
    for (int index=0;index<numResolutions;++index){
        changed.push_back(vector<tuple<int,int> >());
        bproExistence.push_back(vector<vector<bool> >());
        bproExistence[index].resize(get<0>(numOffsets[index]));
        for (int i=0;i<get<0>(numOffsets[index]);++i){
            bproExistence[index][i].resize(get<1>(numOffsets[index]));
            for (int j=0;j<get<1>(numOffsets[index]);++j){
                bproExistence[index][i][j]=true;
            }
        }
        //threePointExistence.push_back(dense_hash_map<long long,int>());
        //threePointExistence.back().set_empty_key(numThreePointOffsets+1);
        //threePointExistence.back().resize(100000);
    }
    
    neighborSize = param->getNeighborSize();
    vector<tuple<int,int> > fullNeighborOffsets;
    for (int xDelta=-neighborSize;xDelta<0;++xDelta){
        for (int yDelta=-neighborSize;yDelta<=neighborSize;++yDelta){
            fullNeighborOffsets.push_back(make_tuple(xDelta,yDelta));
        }
    }
    for (int yDelta=-neighborSize;yDelta<0;++yDelta){
        fullNeighborOffsets.push_back(make_tuple(0,yDelta));
    }
    vector<tuple<int,int> > extraNeighborOffsets;
    extraNeighborOffsets.push_back(make_tuple(0,-1));
    for (int xDelta=-neighborSize;xDelta<0;++xDelta){
        extraNeighborOffsets.push_back(make_tuple(xDelta,neighborSize));
    }
    fullNeighbors = new vector<vector<vector<unsigned short> > >(210);
    extraNeighbors = new vector<vector<vector<unsigned short> > >(210);
    for (unsigned short row=0; row<210;++row){
        fullNeighbors->at(row).resize(160);
        extraNeighbors->at(row).resize(160);
        for (unsigned short column=0; column<160;++column){
            for (auto it = fullNeighborOffsets.begin();it!=fullNeighborOffsets.end();++it){
                unsigned short neighborX = row + get<0>(*it);
                unsigned short neighborY = column + get<1>(*it);
                if (neighborX>=0 && neighborX<210 && neighborY>=0 && neighborY<160){
                    fullNeighbors->at(row)[column].push_back(neighborX*160+neighborY);
                }
            }
            
            for (auto it = extraNeighborOffsets.begin();it!=extraNeighborOffsets.end();++it){
                unsigned short neighborX = row + get<0>(*it);
                unsigned short neighborY = column + get<1>(*it);
                if (neighborX>=0 && neighborX<210 && neighborY>=0 && neighborY<160){
                    extraNeighbors->at(row)[column].push_back(neighborX*160+neighborY);
                }
            }
        }
    }
    previousBlobs.clear();
}

BlobTimeFeatures::~BlobTimeFeatures(){
    delete fullNeighbors;
    delete extraNeighbors;
}

void BlobTimeFeatures::getBlobs(const ALEScreen &screen){
    int screenWidth = 160;
    int screenHeight = 210;
    
    
    vector<int> screenPixels(screenHeight*screenWidth,-1);
    
    vector<Disjoint_Set_Element> disjoint_set;
    vector<unordered_set<int> > blobIndices(numColors,unordered_set<int>());
    vector<int> route;
    
    vector<vector<vector<unsigned short> > >* neighbors;
    
    for (int x=0;x<screenHeight;x++){
        for (int y=0;y<screenWidth;y++){
            int color = screen.get(x,y);
            color = color >>colorMultiplier;
            if (y>0 && color == screen.get(x,y-1)>>colorMultiplier){
                neighbors = extraNeighbors;
            }else{
                neighbors = fullNeighbors;
            }
            unsigned short currentIndex = x*screenWidth + y;
            int currentRoot = screenPixels[currentIndex];
            
            for (auto it=neighbors->at(x).at(y).begin();it!=neighbors->at(x).at(y).end();++it){
                int neighborRoot = screenPixels[*it];
                if (color == disjoint_set[neighborRoot].color){
                    //get the true root
                    route.clear();
                    while (disjoint_set[neighborRoot].parent!=neighborRoot){
                        route.push_back(neighborRoot);
                        neighborRoot = disjoint_set[neighborRoot].parent;
                    }
                    
                    //maintain the disjoint_set
                    for (auto itt=route.begin();itt!=route.end();++itt){
                        disjoint_set[*itt].parent = neighborRoot;
                    }
                    
                   
                    
                    if (neighborRoot!=currentRoot){
                        auto blobNeighbor  = &disjoint_set[neighborRoot];
                        //case 1: current pixel does not belong to any blob
                        if (currentRoot==-1){
                            currentRoot=neighborRoot;
                            blobNeighbor->rowDown = x;
                            if (y < blobNeighbor->columnLeft){
                                blobNeighbor->columnLeft = y;
                            }else if (y > blobNeighbor->columnRight){
                                blobNeighbor->columnRight = y;
                            }
                            blobNeighbor->size+=1;
                            //case 2: current pixel belongs to two blobs
                        }else{
                            auto currentBlob = &disjoint_set[currentRoot];
                            if (blobNeighbor->size>currentBlob->size){
                                currentBlob->parent = neighborRoot;
                                blobIndices[color].erase(currentRoot);
                                updateRepresentatiePixel(x,y,blobNeighbor,currentBlob);
                                currentRoot = neighborRoot;
                            }else{
                                blobNeighbor->parent = currentRoot;
                                blobIndices[color].erase(neighborRoot);
                                updateRepresentatiePixel(x,y,currentBlob,blobNeighbor);
                            }
                        }
                    }
                }
            }
            screenPixels[currentIndex] = currentRoot;
            
            //current pixel is the first pixel for a new blob
            if (screenPixels[currentIndex]==-1){
                Disjoint_Set_Element element;
                element.columnLeft = y; element.columnRight = y;
                element.rowUp = x; element.rowDown = x;
                element.size = 1;
                element.parent = disjoint_set.size();
                screenPixels[currentIndex] = disjoint_set.size();
                element.color = color;
                blobIndices[color].insert(disjoint_set.size());
                disjoint_set.push_back(element);
            }
            
        }
    }
    
    //get all the blobs
    for (int color = 0;color<numColors;++color){
        for (auto index=blobIndices[color].begin();index!=blobIndices[color].end();++index){
            int x = (disjoint_set[*index].rowUp+disjoint_set[*index].rowDown)/2;
            int y = (disjoint_set[*index].columnLeft+disjoint_set[*index].columnRight)/2;
            if (blobs[color].size()==0){
                blobActiveColors.push_back(color);
            }
            blobs[color].push_back(make_tuple(x,y));
        }
    }
}

void BlobTimeFeatures::addRelativeFeaturesIndices(vector<long long>& features){
    for (int index1=0;index1<blobActiveColors.size();++index1){
        int c1 = blobActiveColors[index1];
        for (auto k=blobs[c1].begin();k!=blobs[c1].end();++k){
            for (auto h=blobs[c1].begin();h!=blobs[c1].end();++h){
                
                for (int index=0;index<numResolutions;++index){
                    int rowDelta = get<0>(*k)/get<0>(resolutions[index])-get<0>(*h)/get<0>(resolutions[index]);
                    int columnDelta = get<1>(*k)/get<1>(resolutions[index])-get<1>(*h)/get<1>(resolutions[index]);
                    bool newBproFeature = false;
                    if (rowDelta>0){
                        newBproFeature = true;
                    }else if (rowDelta==0 && columnDelta >=0){
                        newBproFeature = true;
                    }
                    rowDelta += get<0>(numBlocks[index])-1;
                    columnDelta += get<1>(numBlocks[index])-1;
                    long long bproIndex = (numColors+numColors-c1+1)*c1/2*get<0>(numOffsets[index])*get<1>(numOffsets[index])+rowDelta*get<1>(numOffsets[index])+columnDelta;
                    tuple<int,int> pos (rowDelta,columnDelta);
                    if (newBproFeature && bproExistence[index][rowDelta][columnDelta]){
                        changed[index].push_back(pos);
                        bproExistence[index][rowDelta][columnDelta]=false;
                        features.push_back(baseBpro[index]+bproIndex);
                    }
                    
                    //add three point feature
                    /*if (previousBlobs.size()>0){
                        addThreePointOffsetsIndices(features,pos,*k,bproIndex);
                    }*/

                }
                
            }
        }
        resetBproExistence();
        //resetThreePointExistence();
        
        for (int index2=index1+1;index2<blobActiveColors.size();++index2){
            int c2 = blobActiveColors[index2];
            for (auto it1=blobs[c1].begin();it1!=blobs[c1].end();++it1){
                for (auto it2=blobs[c2].begin();it2!=blobs[c2].end();++it2){
                    
                    for (int index=0;index<numResolutions;++index){
                        int rowDelta = get<0>(*it1)/get<0>(resolutions[index])-get<0>(*it2)/get<0>(resolutions[index])+get<0>(numBlocks[index])-1;
                        int columnDelta = get<1>(*it1)/get<1>(resolutions[index])-get<1>(*it2)/get<1>(resolutions[index])+get<1>(numBlocks[index])-1;
                        long long bproIndex = (numColors+numColors-c1+1)*c1/2*get<0>(numOffsets[index])*get<1>(numOffsets[index])+(c2-c1)*get<0>(numOffsets[index])*get<1>(numOffsets[index])+rowDelta*get<1>(numOffsets[index])+columnDelta;
                        tuple<int,int> pos(rowDelta,columnDelta);
                        if (bproExistence[index][rowDelta][columnDelta]){
                            changed[index].push_back(pos);
                            bproExistence[index][rowDelta][columnDelta]=false;
                            features.push_back(baseBpro[index]+bproIndex);
                        }
                        
                        //add three point feature
                        /*if (previousBlobs.size()>0){
                            addThreePointOffsetsIndices(features,pos,*it1,bproIndex);
                        }*/

                        
                    }
                }
            }
            resetBproExistence();
            //resetThreePointExistence();
        }
    }
}

void BlobTimeFeatures::getBasicFeatures(vector<long long>& features){
    vector<bool> basicNotExistence(numBasicFeatures,true);
    for (unsigned short i=0;i<blobActiveColors.size();i++){
        int color = blobActiveColors[i];
        for (auto itt = blobs[color].begin();itt!=blobs[color].end();++itt){
            int x = get<0>(*itt);
            int y = get<1>(*itt);
            for (int index=0;index<numResolutions;++index){
                long long featureIndex = baseBasic[index]+x/get<0>(resolutions[index])*get<1>(numBlocks[index])+y/get<1>(resolutions[index]);
                if (basicNotExistence[featureIndex]){
                    features.push_back(featureIndex);
                    basicNotExistence[featureIndex] = false;
                }
            }
        }
    }
}

void BlobTimeFeatures::addTimeDimensionalOffsets(vector<long long>& features){
    for (int index1=0;index1<previousBlobActiveColors.size();++index1){
        int c1 = previousBlobActiveColors[index1];
        for (int index2=0;index2<blobActiveColors.size();++index2){
            int c2 = blobActiveColors[index2];
            
            for (auto it1=previousBlobs[c1].begin();it1 !=previousBlobs[c1].end();++it1){
                for (auto it2=blobs[c2].begin();it2 != blobs[c2].end();++it2){
                    
                    for (int index=0;index<numResolutions;++index){
                        int rowDelta =get<0>(*it1)/get<0>(resolutions[index])-get<0>(*it2)/get<0>(resolutions[index])+get<0>(numBlocks[index])-1;
                        int columnDelta = get<1>(*it1)/get<1>(resolutions[index])-get<1>(*it2)/get<1>(resolutions[index])+get<1>(numBlocks[index])-1;
                        if (bproExistence[index][rowDelta][columnDelta]){
                            tuple<int,int> pos(rowDelta,columnDelta);
                            changed[index].push_back(pos);
                            bproExistence[index][rowDelta][columnDelta]=false;
                            features.push_back(baseTime[index]+c1*numColors*get<0>(numOffsets[index])*get<1>(numOffsets[index])+c2*get<0>(numOffsets[index])*get<1>(numOffsets[index])+rowDelta*get<1>(numOffsets[index])+columnDelta);
                        }
                    }
                }
            }
            resetBproExistence();
           
        }
    }
    
}

void BlobTimeFeatures::addThreePointOffsetsIndices(vector<long long>& features, tuple<int,int>& offset, tuple<int,int>& p1, long long& bproIndex){
    for (int index3=0;index3<previousBlobActiveColors.size();++index3){
        int c3 = previousBlobActiveColors[index3];
        for (vector<tuple<int,int> >::iterator it = previousBlobs[c3].begin();it!=previousBlobs[c3].end();it++){
            for (int index=0;index<numResolutions;++index){
                int rowDelta = get<0>(p1)-get<0>(*it)+get<0>(numBlocks[index])-1;
                int columnDelta = get<1>(p1)-get<1>(*it)+get<0>(numBlocks[index])-1;
                long long threePointIndex = bproIndex*numColors*get<0>(numOffsets[index])*get<1>(numOffsets[index])+c3*get<0>(numOffsets[index])*get<1>(numOffsets[index])+rowDelta*get<1>(numOffsets[index])+columnDelta;
                if (threePointExistence[index][threePointIndex]==0){
                    threePointExistence[index][threePointIndex]=1;
                    features.push_back(baseThreePoint[index]+threePointIndex);
                }
            }
        }
    }
}


void BlobTimeFeatures::getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<long long>& features){
    
    blobs.clear();
    blobs.resize(numColors);
    blobActiveColors.clear();
    getBlobs(screen);
    
    /*long long numBlobsForPrint = 0;
    for (auto it=blobs.begin();it!=blobs.end();++it){
        numBlobsForPrint+=it->size();
    }
    cout<<numBlobsForPrint<<endl;*/
    getBasicFeatures(features);
    addRelativeFeaturesIndices(features);
    if (previousBlobs.size()>0){
        addTimeDimensionalOffsets(features);
    }
    features.push_back(numBasicFeatures+numRelativeFeatures + numTimeDimensionalOffsets);
    previousBlobs = blobs;
    previousBlobActiveColors = blobActiveColors;
}


long long BlobTimeFeatures::getNumberOfFeatures(){
    return numBasicFeatures+numRelativeFeatures + numTimeDimensionalOffsets+1;
}

void BlobTimeFeatures::resetBproExistence(){
    for (int index = 0; index<numResolutions;++index){
        for (vector<tuple<int,int> >::iterator it = changed[index].begin();it!=changed[index].end();++it){
            bproExistence[index][get<0>(*it)][get<1>(*it)]=true;
        }
        changed[index].clear();
    }
}

void BlobTimeFeatures::resetThreePointExistence(){
    for (int index = 0; index<numResolutions;++index){
        threePointExistence[index].clear();
    }
}

int BlobTimeFeatures::getPowerTwoOffset(int rawDelta){
    int multiplier = 1;
    if (rawDelta<0){
        multiplier = -1;
    }
    rawDelta = abs(rawDelta)+1;
    return ceil(log2(rawDelta))*multiplier;
}

void BlobTimeFeatures::updateRepresentatiePixel(int& x, int& y, Disjoint_Set_Element* root, Disjoint_Set_Element* other){
    root->rowUp = min(root->rowUp,other->rowUp);
    root->rowDown = x;
    root->columnLeft = min(root->columnLeft,other->columnLeft);
    root->columnRight = max(root->columnRight,other->columnRight);
    root->size += other->size;
}

void BlobTimeFeatures::clearCash(){
    previousBlobs.clear();
    previousBlobActiveColors.clear();
}
