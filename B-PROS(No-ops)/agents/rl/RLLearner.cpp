#ifndef MATHEMATICS_H
#define MATHEMATICS_H
#include "../../common/Mathematics.hpp"
#endif

#ifndef RL_LEARNER_H
#define RL_LEARNER_H
#include "RLLearner.hpp"
#include <random>
#endif

RLLearner::RLLearner(ALEInterface& ale, Parameters *param, int seed){
	randomActionTaken   = 0;

	gamma               = param->getGamma();
	epsilon             = param->getEpsilon();
	toUseOnlyRewardSign = param->getUseRewardSign();
	toBeOptimistic      = param->getOptimisticInitialization();
	
	episodeLength       = param->getEpisodeLength();
	numEpisodesEval     = param->getNumEpisodesEval();
	totalNumberOfFramesToLearn = param->getLearningLength();

	//Get the number of effective actions:
	if(param->isMinimalAction()){
		actions = ale.getMinimalActionSet();
	}
	else{
		actions = ale.getLegalActionSet();
	}
	numActions = actions.size();
    agentRand = param->getRNG();
    agentRand->seed(seed);
}

int RLLearner::epsilonGreedy(vector<float> &QValues){
	randomActionTaken = 0;

	int action = Mathematics::argmax(QValues,agentRand);
	//With probability epsilon: a <- random action in A(s)
	int random = (*agentRand)();
	if((random % int(nearbyint(1.0/epsilon))) == 0) {
        //if((rand()%int(1.0/epsilon)) == 0){
		randomActionTaken = 1;
		action = (*agentRand)() % numActions;
	}
    return action;
}

/**
 * The first parameter is the one that is used by Sarsa. The second is used to
 * pass aditional information to the running algorithm (like 'real score' if one
 * is using a surrogate reward function).
 */
void RLLearner::act(ALEInterface& ale, int action, vector<float> &reward){
	double r_alg = 0.0, r_real = 0.0;
	
	r_real = ale.act(actions[action]);
	if(toUseOnlyRewardSign){
		if(r_real > 0){ 
			r_alg = 1.0;
		}
		else if(r_real < 0){
			r_alg = -1.0;
		}
	//Normalizing reward according to the first
	//reward, Marc did this in his JAIR paper:
	} else{
		if(r_real != 0.0){
			if(!sawFirstReward){
				firstReward = std::abs(r_real);
				sawFirstReward = 1;
			}
		}
		if(sawFirstReward){
			if(toBeOptimistic){
				r_alg = (r_real - firstReward)/firstReward + gamma;
			}
			else{
				r_alg = r_real/firstReward;	
			}
		}
		else{
			if(toBeOptimistic){
				r_alg = gamma - 1.0;
			}
		}
	}
	reward[0] = r_alg;
	reward[1] = r_real;
	//If doing optimistic initialization, to avoid the agent
	//to "die" soon to avoid -1 as reward at each step, when
	//the agent dies we give him -1 for each time step remaining,
	//this would be the worst case ever...
	if(ale.game_over() && toBeOptimistic){
		int missedSteps = episodeLength - ale.getEpisodeFrameNumber() + 1;
		double penalty = pow(gamma, missedSteps) - 1;
		reward[0] -= penalty;
	}
}
