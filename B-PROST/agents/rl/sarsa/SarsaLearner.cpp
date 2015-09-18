/****************************************************************************************
 ** Implementation of Sarsa(lambda). It implements Fig. 8.8 (Linear, gradient-descent
 ** Sarsa(lambda)) from the book "R. Sutton and A. Barto; Reinforcement Learning: An
 ** Introduction. 1st edition. 1988."
 ** Some updates are made to make it more efficient, as not iterating over all features.
 **
 ** TODO: Make it as efficient as possible.
 **
 ** Author: Marlos C. Machado
 ***************************************************************************************/

#ifndef TIMER_H
#define TIMER_H
#include "../../../common/Timer.hpp"
#endif
#include "SarsaLearner.hpp"
#include <stdio.h>
#include <math.h>
#include <set>
using namespace std;
//using google::dense_hash_map;

SarsaLearner::SarsaLearner(ALEInterface& ale, Features *features, Parameters *param,int seed) : RLLearner(ale, param,seed) {
    
    totalNumberFrames = 0.0;
    maxFeatVectorNorm = 1;
    saveThreshold =0;
    
    delta = 0.0;
    alpha = param->getAlpha();
    learningRate = alpha;
    lambda = param->getLambda();
    numGroups = 0;
    traceThreshold = param->getTraceThreshold();
    numFeatures = features->getNumberOfFeatures();
    toSaveCheckPoint = param->getToSaveCheckPoint();
    saveWeightsEveryXFrames = param->getFrequencySavingWeights();
    pathWeightsFileToLoad = param->getPathToWeightsFiles();
    randomNoOp = param->getRandomNoOp();
    noOpMax = param->getNoOpMax();
    numStepsPerAction = param->getNumStepsPerAction();
    
    for(int i = 0; i < numActions; i++){
        //Initialize Q;
        Q.push_back(0);
        Qnext.push_back(0);
        //Initialize e:
        e.push_back(vector<float>());
        w.push_back(vector<float>());
        nonZeroElig.push_back(vector<long long>());
    }
    episodePassed = 0;
    featureTranslate.clear();
    featureTranslate.max_load_factor(0.5);
    if(toSaveCheckPoint){
        checkPointName = param->getCheckPointName();
        //load CheckPoint
        ifstream checkPointToLoad;
        string checkPointLoadName = checkPointName+"-checkPoint.txt";
        checkPointToLoad.open(checkPointLoadName.c_str());
        if (checkPointToLoad.is_open()){
            loadCheckPoint(checkPointToLoad);
            remove(checkPointLoadName.c_str());
        }
        saveThreshold = (totalNumberFrames/saveWeightsEveryXFrames)*saveWeightsEveryXFrames;
        ofstream learningConditionFile;
        nameForLearningCondition = checkPointName+"-learningCondition-Frames"+to_string(saveThreshold)+"-finished.txt";
        string previousNameForLearningCondition =checkPointName +"-learningCondition.txt";
        rename(previousNameForLearningCondition.c_str(), nameForLearningCondition.c_str());
        saveThreshold+=saveWeightsEveryXFrames;
        learningConditionFile.open(nameForLearningCondition, ios_base::app);
        learningConditionFile.close();
    }
}

SarsaLearner::~SarsaLearner(){}

void SarsaLearner::updateQValues(vector<long long> &Features, vector<float> &QValues){
    unsigned long long featureSize = Features.size();
    for(int a = 0; a < numActions; ++a){
        float sumW = 0;
        for(unsigned long long i = 0; i < featureSize; ++i){
            sumW = sumW + w[a][Features[i]]*groups[Features[i]].numFeatures;
        }
        QValues[a] = sumW;
    }
}

void SarsaLearner::updateReplTrace(int action, vector<long long> &Features){
    //e <- gamma * lambda * e
    for(unsigned int a = 0; a < nonZeroElig.size(); a++){
        long long numNonZero = 0;
        for(unsigned long long i = 0; i < nonZeroElig[a].size(); i++){
            long long idx = nonZeroElig[a][i];
            //To keep the trace sparse, if it is
            //less than a threshold it is zero-ed.
            e[a][idx] = gamma * lambda * e[a][idx];
            if(e[a][idx] < traceThreshold){
                e[a][idx] = 0;
            }
            else{
                nonZeroElig[a][numNonZero] = idx;
                numNonZero++;
            }
        }
        nonZeroElig[a].resize(numNonZero);
    }
    
    //For all i in Fa:
    for(unsigned int i = 0; i < F.size(); i++){
        long long idx = Features[i];
        //If the trace is zero it is not in the vector
        //of non-zeros, thus it needs to be added
        if(e[action][idx] == 0){
            nonZeroElig[action].push_back(idx);
        }
        e[action][idx] = 1;
    }
}

void SarsaLearner::updateAcumTrace(int action, vector<long long> &Features){
    //e <- gamma * lambda * e
    for(unsigned int a = 0; a < nonZeroElig.size(); a++){
        long long numNonZero = 0;
        for(unsigned int i = 0; i < nonZeroElig[a].size(); i++){
            long long idx = nonZeroElig[a][i];
            //To keep the trace sparse, if it is
            //less than a threshold it is zero-ed.
            e[a][idx] = gamma * lambda * e[a][idx];
            if(e[a][idx] < traceThreshold){
                e[a][idx] = 0;
            }
            else{
                nonZeroElig[a][numNonZero] = idx;
                numNonZero++;
            }
        }
        nonZeroElig[a].resize(numNonZero);
    }
    
    //For all i in Fa:
    for(unsigned int i = 0; i < F.size(); i++){
        long long idx = Features[i];
        //If the trace is zero it is not in the vector
        //of non-zeros, thus it needs to be added
        if(e[action][idx] == 0){
            nonZeroElig[action].push_back(idx);
        }
        e[action][idx] += 1;
    }
}

void SarsaLearner::sanityCheck(){
    for(int i = 0; i < numActions; i++){
        if(fabs(Q[i]) > 10e7 || Q[i] != Q[i] /*NaN*/){
            printf("It seems your algorithm diverged!\n");
            exit(0);
        }
    }
}

//To do: we do not want to save weights that are zero
void SarsaLearner::saveCheckPoint(int episode, int totalNumberFrames, vector<float>& episodeResults,int& frequency,vector<int>& episodeFrames, vector<double>& episodeFps){
    ofstream learningConditionFile;
    string newNameForLearningCondition = checkPointName+"-learningCondition-Frames"+to_string(saveThreshold)+"-writing.txt";
    int renameReturnCode = rename(nameForLearningCondition.c_str(),newNameForLearningCondition.c_str());
    if (renameReturnCode == 0){
        nameForLearningCondition = newNameForLearningCondition;
        learningConditionFile.open(nameForLearningCondition.c_str(), ios_base::app);
        int numEpisode = episodeResults.size();
        for (int index = 0;index<numEpisode;index++){
            learningConditionFile <<"Episode "<<episode-numEpisode+1+index<<": "<<episodeResults[index]<<" points,  "<<episodeFrames[index]<<" frames,  "<<episodeFps[index]<<" fps."<<endl;
        }
        episodeResults.clear();
        episodeFrames.clear();
        episodeFps.clear();
        learningConditionFile.close();
        newNameForLearningCondition.replace(newNameForLearningCondition.end()-11,newNameForLearningCondition.end()-4,"finished");
        rename(nameForLearningCondition.c_str(),newNameForLearningCondition.c_str());
        nameForLearningCondition = newNameForLearningCondition;
    }
    
    //write parameters checkPoint
    string currentCheckPointName = checkPointName+"-checkPoint-Frames"+to_string(saveThreshold)+"-writing.txt";
    ofstream checkPointFile;
    checkPointFile.open(currentCheckPointName.c_str());
    checkPointFile<<(*agentRand)<<endl;
    checkPointFile<<totalNumberFrames<<endl;
    checkPointFile << episode<<endl;
    checkPointFile << firstReward<<endl;
    checkPointFile << maxFeatVectorNorm<<endl;
    checkPointFile << numGroups<<endl;
    checkPointFile << featureTranslate.size()<<endl;
    vector<int> nonZeroWeights;
    for (unsigned long long groupIndex=0; groupIndex<numGroups;++groupIndex){
        nonZeroWeights.clear();
        for (unsigned long long a=0; a<w.size();a++){
            if (w[a][groupIndex]!=0){
                nonZeroWeights.push_back(a);
            }
        }
        checkPointFile<<nonZeroWeights.size();
        for (int i=0;i<nonZeroWeights.size();++i){
            int action = nonZeroWeights[i];
            checkPointFile<<" "<<action<<" "<<w[action][groupIndex];
        }
        checkPointFile<<"\t";
    }
    checkPointFile << endl;
    
    for (auto it=featureTranslate.begin(); it!=featureTranslate.end();++it){
        checkPointFile<<it->first<<" "<<it->second<<"\t";
    }
    checkPointFile<<endl;
    checkPointFile.close();
    
    string previousVersionCheckPoint = checkPointName+"-checkPoint-Frames"+to_string(saveThreshold-saveWeightsEveryXFrames)+"-finished.txt";
    if((saveThreshold-saveWeightsEveryXFrames)%50000000 != 0){
    	remove(previousVersionCheckPoint.c_str());
    }
    string oldCheckPointName = currentCheckPointName;
    currentCheckPointName.replace(currentCheckPointName.end()-11,currentCheckPointName.end()-4,"finished");
    rename(oldCheckPointName.c_str(),currentCheckPointName.c_str());
    
}

void SarsaLearner::loadCheckPoint(ifstream& checkPointToLoad){
    checkPointToLoad >> (*agentRand);
    checkPointToLoad >> totalNumberFrames;
    while (totalNumberFrames<1000){
        checkPointToLoad >> totalNumberFrames;
    }
    checkPointToLoad >> episodePassed;
    checkPointToLoad >> firstReward;
    checkPointToLoad >> maxFeatVectorNorm;
    learningRate = alpha / float(maxFeatVectorNorm);
    checkPointToLoad >> numGroups;
    long long numberOfFeaturesSeen;
    checkPointToLoad >> numberOfFeaturesSeen;
    for (unsigned long long index=0;index<numGroups;++index){
        Group agroup;
        agroup.numFeatures = 0;
        agroup.features.clear();
        groups.push_back(agroup);
    }
    for (unsigned a =0;a<w.size();a++){
        w[a].resize(numGroups,0.00);
        e[a].resize(numGroups,0.00);
    }
    int action;
    float weight;
    int numNonZeroWeights;
    for (unsigned long long groupIndex=0; groupIndex<numGroups;++groupIndex){
        checkPointToLoad >> numNonZeroWeights;
        for (unsigned int i=0; i<numNonZeroWeights;++i){
            checkPointToLoad >> action; checkPointToLoad >> weight;
            w[action][groupIndex] = weight;
        }
    }
    
    long long featureIndex;
    long long featureToGroup;
    while (checkPointToLoad >> featureIndex && checkPointToLoad >> featureToGroup){
        featureTranslate[featureIndex] = featureToGroup;
        groups[featureToGroup-1].numFeatures+=1;
    }
    checkPointToLoad.close();
}

void SarsaLearner::learnPolicy(ALEInterface& ale, Features *features){
    
    struct timeval tvBegin, tvEnd, tvDiff;
    vector<float> reward;
    double elapsedTime;
    double cumReward = 0, prevCumReward = 0;
    sawFirstReward = 0; firstReward = 1.0;
    vector<float> episodeResults;
    vector<int> episodeFrames;
    vector<double> episodeFps;
    
    long long trueFeatureSize = 0;
    long long trueFnextSize = 0;
    
    //Repeat (for each episode):
    //This is going to be interrupted by the ALE code since I set max_num_frames beforehand
    for(int episode = episodePassed+1; totalNumberFrames < totalNumberOfFramesToLearn; episode++){
        //random no-op
        unsigned int noOpNum = 0;
        if (randomNoOp){
            noOpNum = (*agentRand)()%(noOpMax)+1;
            for (int i=0;i<noOpNum;++i){
                ale.act(actions[0]);
            }
        }
        
        //We have to clean the traces every episode:
        for(unsigned int a = 0; a < nonZeroElig.size(); a++){
            for(unsigned long long i = 0; i < nonZeroElig[a].size(); i++){
                long long idx = nonZeroElig[a][i];
                e[a][idx] = 0.0;
            }
            nonZeroElig[a].clear();
        }
        
        F.clear();
        features->getActiveFeaturesIndices(ale.getScreen(), ale.getRAM(), F);
        trueFeatureSize = F.size();
        groupFeatures(F);
        updateQValues(F, Q);
        
        currentAction = epsilonGreedy(Q,episode);
        gettimeofday(&tvBegin, NULL);
        int lives = ale.lives();
        //Repeat(for each step of episode) until game is over:
        //This also stops when the maximum number of steps per episode is reached
        while(!ale.game_over()){
            reward.clear();
            reward.push_back(0.0);
            reward.push_back(0.0);
            updateQValues(F, Q);
            updateReplTrace(currentAction, F);
            
            sanityCheck();
            //Take action, observe reward and next state:
            act(ale, currentAction, reward);
            cumReward  += reward[1];
            if(!ale.game_over()){
                //Obtain active features in the new state:
                Fnext.clear();
                features->getActiveFeaturesIndices(ale.getScreen(), ale.getRAM(), Fnext);
                trueFnextSize = Fnext.size();
                groupFeatures(Fnext);
                updateQValues(Fnext, Qnext);     //Update Q-values for the new active features
                nextAction = epsilonGreedy(Qnext,episode);
            }
            else{
                nextAction = 0;
                for(unsigned int i = 0; i < Qnext.size(); i++){
                    Qnext[i] = 0;
                }
            }
            //To ensure the learning rate will never increase along
            //the time, Marc used such approach in his JAIR paper
            if (trueFeatureSize > maxFeatVectorNorm){
                maxFeatVectorNorm = trueFeatureSize;
                learningRate = alpha/maxFeatVectorNorm;
            }
            delta = reward[0] + gamma * Qnext[nextAction] - Q[currentAction];
            
            //Update weights vector:
            for(unsigned int a = 0; a < nonZeroElig.size(); a++){
                for(unsigned int i = 0; i < nonZeroElig[a].size(); i++){
                    long long idx = nonZeroElig[a][i];
                    w[a][idx] = w[a][idx] + learningRate * delta * e[a][idx];
                }
            }
            F = Fnext;
            trueFeatureSize = trueFnextSize;
            currentAction = nextAction;
        }
        gettimeofday(&tvEnd, NULL);
        timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
        elapsedTime = double(tvDiff.tv_sec) + double(tvDiff.tv_usec)/1000000.0;
        
        double fps = double(ale.getEpisodeFrameNumber())/elapsedTime;
        printf("episode: %d,\t%.0f points,\tavg. return: %.1f,\t%d frames,\t%.0f fps\n",
               episode, cumReward - prevCumReward, (double)cumReward/(episode),
               ale.getEpisodeFrameNumber(), fps);
        episodeResults.push_back(cumReward-prevCumReward);
        episodeFrames.push_back(ale.getEpisodeFrameNumber());
        episodeFps.push_back(fps);
        totalNumberFrames += ale.getEpisodeFrameNumber()-noOpNum*numStepsPerAction;
        prevCumReward = cumReward;
        features->clearCash();
        ale.reset_game();
        if(toSaveCheckPoint && totalNumberFrames>saveThreshold){
            saveCheckPoint(episode,totalNumberFrames,episodeResults,saveWeightsEveryXFrames,episodeFrames,episodeFps);
            saveThreshold+=saveWeightsEveryXFrames;
        }
    }
}

void SarsaLearner::evaluatePolicy(ALEInterface& ale, Features *features){
    double reward = 0;
    double cumReward = 0;
    double prevCumReward = 0;
    struct timeval tvBegin, tvEnd, tvDiff;
    double elapsedTime;
    
    std::string oldName = checkPointName+"-Result-writing.txt";
    std::string newName = checkPointName+"-Result-finished.txt";
    std::ofstream resultFile;
    resultFile.open(oldName.c_str());
    
    //Repeat (for each episode):
    for(int episode = 1; episode < numEpisodesEval; episode++){
        //Repeat(for each step of episode) until game is over:
        gettimeofday(&tvBegin, NULL);
        //random no-op
        unsigned int noOpNum;
        if (randomNoOp){
            noOpNum = (*agentRand)()%(noOpMax)+1;
            for (int i=0;i<noOpNum;++i){
                ale.act(actions[0]);
            }
        }
        
        for(int step = 0; !ale.game_over() && step < episodeLength; step++){
            //Get state and features active on that state:
            F.clear();
            features->getActiveFeaturesIndices(ale.getScreen(), ale.getRAM(), F);
            groupFeatures(F);
            updateQValues(F, Q);       //Update Q-values for each possible action
            currentAction = epsilonGreedy(Q);
            //Take action, observe reward and next state:
            reward = ale.act(actions[currentAction]);
            cumReward  += reward;
        }
        gettimeofday(&tvEnd, NULL);
        timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
        elapsedTime = double(tvDiff.tv_sec) + double(tvDiff.tv_usec)/1000000.0;
        double fps = double(ale.getEpisodeFrameNumber())/elapsedTime;
        
        resultFile<<"Episode "<<episode<<": "<<cumReward-prevCumReward<<std::endl;
        printf("episode: %d,\t%.0f points,\tavg. return: %.1f,\t%d frames,\t%.0f fps\n",
               episode, (cumReward-prevCumReward), (double)cumReward/(episode), ale.getEpisodeFrameNumber(), fps);
        features->clearCash();
        ale.reset_game();
        prevCumReward = cumReward;
    }
    resultFile<<"Average: "<<(double)cumReward/numEpisodesEval<<std::endl;
    resultFile.close();
    rename(oldName.c_str(),newName.c_str());
    
}

void SarsaLearner::groupFeatures(vector<long long>& activeFeatures){
    vector<long long> activeGroupIndices;
    
    int newGroup = 0;
    for (unsigned long long i = 0; i <activeFeatures.size();++i){
        long long featureIndex = activeFeatures[i];
        if (featureTranslate[featureIndex] == 0){
            if (newGroup){
                featureTranslate[featureIndex] = numGroups;
                groups[numGroups-1].numFeatures+=1;
            }else{
                newGroup = 1;
                Group agroup;
                agroup.numFeatures = 1;
                agroup.features.clear();
                groups.push_back(agroup);
                for (unsigned int action=0;action<w.size();++action){
                    w[action].push_back(0.0);
                    e[action].push_back(0.0);
                }
                ++numGroups;
                featureTranslate[featureIndex] = numGroups;
            }
        }else{
            long long groupIndex = featureTranslate[featureIndex]-1;
            auto it = &groups[groupIndex].features;
            if (it->size() == 0){
                activeGroupIndices.push_back(groupIndex);
            }
            it->push_back(featureIndex);
        }
    }
    
    activeFeatures.clear();
    if (newGroup){
        activeFeatures.push_back(groups.size()-1);
    }
    
    for (unsigned long long index=0;index<activeGroupIndices.size();++index){
        long long groupIndex = activeGroupIndices[index];
        if (groups[groupIndex].features.size()!=groups[groupIndex].numFeatures && groups[groupIndex].features.size()!=0){
            Group agroup;
            agroup.numFeatures = groups[groupIndex].features.size();
            agroup.features.clear();
            groups.push_back(agroup);
            ++numGroups;
            for (unsigned long long i =0; i<groups[groupIndex].features.size();++i){
                featureTranslate[groups[groupIndex].features[i]] = numGroups;
            }
            activeFeatures.push_back(numGroups-1);
            for (unsigned a = 0;a<w.size();++a){
                w[a].push_back(w[a][groupIndex]);
                e[a].push_back(e[a][groupIndex]);
                if (e[a].back()>=traceThreshold ){
                    nonZeroElig[a].push_back(numGroups-1);
                }
            }
            groups[groupIndex].numFeatures = groups[groupIndex].numFeatures - groups[groupIndex].features.size();
        }else if(groups[groupIndex].features.size()==groups[groupIndex].numFeatures){
            activeFeatures.push_back(groupIndex);
        }
        groups[groupIndex].features.clear();
        groups[groupIndex].features.shrink_to_fit();
    }
}

void SarsaLearner::saveWeightsToFile(string suffix){
    std::ofstream weightsFile ((nameWeightsFile + suffix).c_str());
    if(weightsFile.is_open()){
        weightsFile << w.size() << " " << w[0].size() << std::endl;
        for(unsigned int i = 0; i < w.size(); i++){
            for(unsigned int j = 0; j < w[i].size(); j++){
                if(w[i][j] != 0){
                    weightsFile << i << " " << j << " " << w[i][j] << std::endl;
                }
            }
        }
        weightsFile.close();
    }
    else{
        printf("Unable to open file to write weights.\n");
    }
}

void SarsaLearner::loadWeights(){
    string line;
    int nActions, nFeatures;
    int i, j;
    double value;
    
    std::ifstream weightsFile (pathWeightsFileToLoad.c_str());
    
    weightsFile >> nActions >> nFeatures;
    assert(nActions == numActions);
    assert(nFeatures == numFeatures);
    
    while(weightsFile >> i >> j >> value){
        w[i][j] = value;
    }
}
