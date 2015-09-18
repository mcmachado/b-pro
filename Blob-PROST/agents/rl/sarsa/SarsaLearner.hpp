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

#ifndef RLLEARNER_H
#define RLLEARNER_H
#include "../RLLearner.hpp"
#endif
#include <vector>
#include <unordered_map>
//#include <sparsehash/dense_hash_map>
using namespace std;
//using google::dense_hash_map;

struct Group{
    long long numFeatures;
    vector<long long> features;
};

class SarsaLearner : public RLLearner{
private:
    float alpha, delta, lambda, traceThreshold;
    float learningRate;
    int currentAction, nextAction;
    long long numFeatures;
    int toSaveWeightsAfterLearning, saveWeightsEveryXFrames, toSaveCheckPoint;
    
    std::string nameWeightsFile, pathWeightsFileToLoad;
    std::string checkPointName;
    std::string nameForLearningCondition;
    int episodePassed;
    int totalNumberFrames;
    long long maxFeatVectorNorm;
    int saveThreshold;
    int randomNoOp;
    int noOpMax;
    int numStepsPerAction;
    
    long long numGroups;
    
    vector<long long> F;					//Set of features active
    vector<long long> Fnext;              //Set of features active in next state
    vector<float> Q;               //Q(a) entries
    vector<float> Qnext;           //Q(a) entries for next action
    vector<vector<float> > e;       //Eligibility trace
    vector<vector<float> > w;     //Theta, weights vector
    vector<vector<long long> >nonZeroElig;//To optimize the implementation
    //vector<vector<long long> > featureSeen;
    unordered_map<long long,long long> featureTranslate;
    vector<Group> groups;
    
    /**
     * Constructor declared as private to force the user to instantiate SarsaLearner
     * informing the parameters to learning/execution.
     */
    SarsaLearner();
    /**
     * This method evaluates whether the Q-values are sound. By unsound I mean huge Q-values (> 10e7)
     * or NaN values. If so, it finishes the execution informing the algorithm has diverged.
     */
    void sanityCheck();
    /**
     * In Sarsa the Q-values (one per action) are updated as the sum of weights for that given action.
     * To avoid writing it more than once on the code, its update was extracted to a separate function.
     * It updates the vector<double> Q assuming that vector<int> F is filled, as it sums just the weights
     * that are active in F.
     */
    void updateQValues(vector<long long> &Features, vector<float> &QValues);
    /**
     * When using Replacing traces, all values not related to the current action are set to 0, while the
     * values for the current action that their features are active are set to 1. The traces decay following
     * the rule: e[action][i] = gamma * lambda * e[action][i]. It is possible to also define thresholding.
     */
    void updateReplTrace(int action, vector<long long> &Features);
    /**
     * When using Replacing traces, all values not related to the current action are set to 0, while the
     * values for the current action that their features are active are added 1. The traces decay following
     * the rule: e[action][i] = gamma * lambda * e[action][i]. It is possible to also define thresholding.
     */
    void updateAcumTrace(int action, vector<long long> &Features);
    /**
     * Prints the weights in a file. Each line will contain a weight.
     */
    void saveWeightsToFile(string suffix="");
    /**
     * Loads the weights saved in a file. Each line will contain a weight.
     */
    void loadWeights();
    void saveCheckPoint(int episode, int totalNumberFrames,  vector<float>& episodeResults, int& frequency, vector<int>& episodeFrames, vector<double>& episodeFps);
    void loadCheckPoint(ifstream& checkPointToLoad);
    void groupFeatures(vector<long long>& activeFeatures);
public:
    SarsaLearner(ALEInterface& ale, Features *features, Parameters *param,int seed);
    /**
     * Implementation of an agent controller. This implementation is Sarsa(lambda).
     *
     * @param ALEInterface& ale Arcade Learning Environment interface: object used to define agents'
     *        actions, obtain simulator's screen, RAM, etc.
     * @param Features *features object that defines what feature function that will be used.
     */
    void learnPolicy(ALEInterface& ale, Features *features);
    /**
     * After the policy was learned it is necessary to evaluate its quality. Therefore, a given number
     * of episodes is run without learning (the vector of weights and the trace are not updated).
     *
     * @param ALEInterface& ale Arcade Learning Environment interface: object used to define agents'
     *        actions, obtain simulator's screen, RAM, etc.
     * @param Features *features object that defines what feature function that will be used.
     */
    void evaluatePolicy(ALEInterface& ale, Features *features);
    /**
     * Destructor, not necessary in this class.
     */
    ~SarsaLearner();
};
