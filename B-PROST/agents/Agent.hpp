/****************************************************************************************
** Abstract class that needs to be implemented by any ALE agent. It defines the method 
** that controls the agent, forcing it to act. By using it we ensure that any approach,
** based in RL, Planning or others can be easily run, following the same pattern.
** 
** Author: Marlos C. Machado
***************************************************************************************/

#ifndef ALE_INTERFACE_H
#define ALE_INTERFACE_H
#include <ale_interface.hpp>
#endif
#ifndef PARAMETERS_H
#define PARAMETERS_H
#include "../common/Parameters.hpp"
#endif
#ifndef FEATURES_H
#define FEATURES_H
#include "../features/Features.hpp"
#endif

class Agent{
	public:
	   /**
 		* Pure virtual method that needs to be implemented by any agent. It contains how the agent
 		* learns to behave. In RL it is the learning part, while in Constant actions, for example,
 		* it learns the best action.
 		*
 		* TODO: it may be useful to return something for the caller, as the total reward or policy. 
 		*       Additionally, it may be important to persist what was learned, maybe with another
 		*       pure virtual method?
 		*
 		* @param ALEInterface& ale Arcade Learning Environment interface: object used to define agents'
 		*        actions, obtain simulator screen, RAM, etc.
 		* @param Features *features object that defines what feature function that will be used by the RL
 		*        agents. It may be null for other approaches as in Planning.
 		*/
		virtual void learnPolicy(ALEInterface& ale, Features *features) = 0;
	   /**
 		* Pure virtual method that needs to be implemented by any agent. Once the agent learned a
 		* policy it executes this policy for a given number of episodes. The policy is stored in
 		* the class' object.
 		*
 		* TODO it may be useful to return something for the caller, as an indicator of performance. 
 		*
 		* @param ALEInterface& ale Arcade Learning Environment interface: object used to define agents'
 		*        actions, obtain simulator screen, RAM, etc.
 		* @param Features *features object that defines what feature function that will be used by the RL
 		*        agents. It may be null for other approaches as in Planning.
 		*/
		virtual void evaluatePolicy(ALEInterface& ale, Features *features) = 0;
		/**
		* Destructor, not necessary in this class.
		*/
		virtual ~Agent(){};
};
