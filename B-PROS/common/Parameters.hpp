/****************************************************************************************
** This class reads parameters from a file informed in the command line and store the
** relevant information. It is ideal to centralize all parameters here, as well as defining
** most of them in a file, to ease the execution of several experiments.
** 
** USEFUL INFORMATION: If one wants to add a new parameter, all he needs to do is to put
**   this parameter in the appropriate .cfg file and then add such parameter in the last
**   lines of the method parseParametersFromConfigFile. You also need to add the
**   parameter in the class Parameters, defining its get and set methods.
**
** Author: Marlos C. Machado
***************************************************************************************/

#include <getopt.h>
#include <string>
#include <vector>

class Parameters{
	private:
		std::string gameBeingPlayed;    //name of the game being played, this is used internally
		std::string romPath;            //rom to be executed, informed in the command line
		std::string configPath;         //path to the file with all the other parameters, informed in the command line
		std::string trajectPath;        //path to the file that will store the trajectory (s, a, r) of the game being played
		std::string pathToBackground;   //path to the file containing the game background
		std::string modelPath;          //path to the file containing the model learned by the logistic regression
		std::string fileWithWeights;    //path to the file that we will write the weights after we are done learning
        std::string checkPointName;     //name to save the temporary local checkPoint
        int toSaveCheckPoint;
		std::string pathToWeightsFiles; //path to the file that we will load the weights from
		double alpha;                   //learning rate
		double gamma;                   //discount factor
		double epsilon;                 //exploration probability
		double lambda;                  //trace
		double traceThreshold;          //threshold to make the trace zero, to avoid very small values
		int seed;                       //seed to be used by the random number generator
		int display;                    //if it should display screen
		int numEpisodesLearn;           //number of episodes to learn
		int numEpisodesEval;            //number of episodes to evaluate learned policy
		int numStepsPerAction;          //number of frames the agent perfoms similarly to speed-up the game
		int episodeLength;              //length of a single episode
		int numRows;                    //number of rows for feature representation
		int numColumns;                 //number of columns for feature representation
		int numColors;                  //colors to be considered in the feature representation of the screen
		int minimalAction;              //use only valid actions for the game or all the Atari legal actions
		int useRewardSign;              //if set the rewards are defined as -1 and +1, ignoring scale
		int subtractBackground;         //whether the background should be removed when generating screen-based features
		int toSaveTrajectory;           //whether we will store a human player trajectory while he is playing a game
		int toUseOptimisticInit;        //whether the algorithm should use optimistic initialization
		int toSaveWeightsAfterLearning; //whether we are going to save the weights learned after the whole process
		int frequencySavingWeights;     //If we are asked to save the weights, We need to know how many frames to wait until saving them again
		int toLoadWeights;              //whether we are going to load an already learned set of weights or not
		int learningLength;             //The number of frames to be learned, in total. DQN uses, for example, 50,000,000.

	   /**
 		* Constructor defined as private to force the use of the constructor 
 		* that receive the commmand line information as parameter.
 		*/
		Parameters();
		/**
 		* Method that prints user-friendly information about the parameters
 		* to be passed by the command line when running the code.
 		*
 		* @param char** input from command line
 		*/
		void printHelp(char** argv);
		/**
 		* Parse parameters read from the command line.
 		*
 		*  --config (-c): the path for the configuration file with all other parameters
 		*  --help   (-h): if one needs help about the available parameters
 		*  --rom    (-r): the path for the rom file containing the game to be played
 		* 
 		* @param int argc number of parameters
 		* @param char** argv input from command line
 		*/
		void readParameters(int argc, char** argv);
		/**
 		* Parse parameters from the configurations file. Once it is defined we extract each of
 		* the relevant parameters, storing them to future use.
 		*
 		* @param std::string cfgFileName path to the configurations file
 		*/
		void parseParametersFromConfigFile(std::string cfgFileName);
		/**
 		* Parse a line from the configuration file returning the pair <ID, Value>.
 		*
 		* @param std::string line the line to be parsed
 		*
 		* @return an empty vector if the line is empty or a comment, otherwise return a vector 
 		*  with two elements, the first being the id and the second, the value.
 		*/
		std::vector<std::string> parseLine(std::string line);
		/**
 		* @param std::string name path to the file that will store the trajectory (s, a, r)
 		*   along the game. This parameter is optional.
 		*/
		void setSaveTrajectoryPath(std::string name);
		/**
		* @param std::string path to the directory with background files
		* @param std::string romFile name of the game that is being played
		*/
		void setPathToBackground(std::string path, std::string romFile);
		/**
 		* @param std::string name path to the rom file to be executed
 		*/
		void setRomPath(std::string name);
		/**
 		* @param std::string name path to the config file to be parsed
 		*/
		void setConfigPath(std::string name);
		/**
		* @param int value that contains the seed that will be used when randomizing
		*/
		void setModelPath(std::string name);
		/**
 		* @param std::string path to the file that will store the learned weights.
 		*/
		void setFileWithWeights(std::string name);
		/**
		* @param int wether we are going to save the learned weights at the end
		*/
		void setToSaveWeightsAfterLearning(int a);
		/**
		* @param int value that contains the seed that will be used when randomizing
		*/		
		void setSeed(std::string name);
		/**
 		* @param double value that represents ALPHA in the config file
 		*/
		void setAlpha(double a);
		/**
 		* @param double value that represents GAMMA in the config file
 		*/
		void setGamma(double a);
		/**
 		* @param double value that represents EPSILON in the config file
 		*/
		void setEpsilon(double a);
		/**
 		* @param double value that represents LAMBDA in the config file
 		*/
		void setLambda(double a);
		/**
 		* @param int (representing double) value that represents DISPLAY in the config file.
 		* If 1 the game will be shown, otherwise it is just run without graphical output.
 		*/
		void setDisplay(int a);
		/**
 		* @param int value that represents NUM_EPISODES in the config file.
 		*/
		void setNumEpisodesLearn(int a);
		/**
 		* @param int value that represents EPISODE_LENGTH_LEARN in the config file.
 		*/
 		void setNumEpisodesEval(int a);
		/**
 		* @param int value that represents EPISODE_LENGTH_EVAL in the config file.
 		*/
		void setEpisodeLength(int a);
		/**
		* @param int value that represents NUM_STEPS_PER_ACTION in the config file.
		*/
		void setNumStepsPerAction(int a);
		/**
 		* @param int value that represents NUM_ROWS in the config file.
 		*/
		void setNumRows(int a);
		/**
 		* @param int value that represents NUM_COLS in the config file.
 		*/
		void setNumColumns(int a);
		/**
 		* @param int value that represents NUM_COLORS in the config file.
 		*/
		void setNumColors(int a);
		/**
 		* @param int (representing bool) value that represents USE_MIN_ACTIONS in the config file.
 		* If 1 we only consider actions that have an effect in the game, otherwise we consider all 18.
 		*/
		void setIsMinimalAction(int a);
		/**
 		* @param double value that represents TRACE_THRESHOLD in the config file.
 		*/
		void setTraceThreshold(double a);
		/**
 		* @param double value that represents USE_REWARD_SIGN in the config file.
 		*/
		void setUseRewardSign(int a);
		/**
 		* @param double value that represents SUBTRACT_BACKGROUND in the config file.
 		*/
		void setSubtractBackground(int a);
		/**
 		* @param int value that represents SAVE_TRAJECTORY in the config file.
 		*/		
		void setToSaveTrajectory(int a);
		/**
 		* @param int value that represents OPTIMISTIC_INIT in the config file.
 		*/		
		void setOptimisticInitialization(int a);
		/**
		* @param int value which means we are going to save the weights at each 'value' episodes
		*/
		void setFrequencySavingWeights(int a);
		/**
		* @param int whether we want or not to load an initial set of weights
		*/		
		void setToLoadWeights(int a);
		/**
		* @param path to the file containing the weights to be loaded
		*/
		void setPathToWeightsFiles(std::string path);
		/**
		* @param learningLength number of frames to be learned in total, e.g. 50,000,000 (DQN).
		*/
		void setLearningLength(int a);
        void setToSaveCheckPoint(int a);
        void setCheckPointName(std::string fileName);
		
	public:
		/**
 		* Constructor receiving the information passed as arguments in the command line.
 		*
 		* @param int argc number of parameters
 		* @param char** argv input from command line
 		*/
		Parameters(int argc, char** argv);
		/**
 		* @return std::string path the file with background information
 		*/
		std::string getPathToBackground();
		/**
 		* @return std::string path to the configuration file that will be/was parsed
 		*/
		std::string getConfigPath();
		/**
 		* @return std::string path to the rom file that will be/was executed as the game
 		*/
		std::string getRomPath();
		/**
 		* @return std::string path to the model file that will be used by the logistic regression
 		*/
		std::string getModelPath();		
		/**
 		* @return std::string path to the file that will store the trajectory (s, a, r)
 		*   along the game. This parameter is optional.
 		*/
		std::string getSaveTrajectoryPath();
		/**
 		* @return std::string path to the file that will store the learned weights.
 		*   This parameter is optional.
 		*/
		std::string getFileWithWeights();
		/**
		* @return whether we have to save the weights after learning or not
		*/
		int getToSaveWeightsAfterLearning();
		/**
		* @return seed used by the randomizer
		*/
		int getSeed();
		/**
 		* @return double value read for ALPHA parameter
 		*/
		double getAlpha();
		/**
 		* @return double value read for GAMMA parameter
 		*/
		double getGamma();
		/**
 		* @return double value read for EPSILON parameter
 		*/
		double getEpsilon();
		/**
 		* @return double value read for LAMBDA parameter
 		*/
		double getLambda();
		/**
 		* @return int (representing bool) value read for DISPLAY parameter
 		* If 1 the game will be shown, otherwise it is just run without graphical output.
 		*/
		int getDisplay();
		/**
 		* @return int value read for NUM_EPISODES parameter
 		*/
		int getNumEpisodesLearn();
		/**
 		* @return int value read for EPISODE_LENGTH_LEARN parameter
 		*/
 		int getNumEpisodesEval();
		/**
 		* @return int value read for EPISODE_LENGTH_EVAL parameter
 		*/
		int getEpisodeLength();
		/*
		* @return int value read for NUM_STEPS_PER_ACTION parameter
		*/
		int getNumStepsPerAction();
		/**
 		* @return int value read for NUM_ROWS parameter
 		*/
		int getNumRows();
		/**
 		* @return int value read for NUM_COLUMNS parameter
 		*/
		int getNumColumns();
		/**
 		* @return int value read for NUM_COLORS parameter
 		*/
		int getNumColors();
		/**
 		* @return int (representing bool) value read for USE_MIN_ACTIONS parameter
 		*/
		int isMinimalAction();
		/**
		* @return double value read for TRACE_THRESHOLD parameter
		*/ 
		double getTraceThreshold();
		/**
		* @return double value read for USE_REWARD_SIGN parameter
		*/ 
		int getUseRewardSign();
		/**
		* @return double value read for SUBTRACT_BACKGROUND parameter
		*/ 
		int getSubtractBackground();
		/**
 		* @return int value read for SAVE_TRAJECTORY parameter
 		*/		
		int getToSaveTrajectory();
		/**
		* @return int value read for OPTIMISTIC_INIT parameter
 		*/
		int getOptimisticInitialization();
		/**
		* @return int value which means we are going to save the weights at each 'value' episodes
		*/
		int getFrequencySavingWeights();
		/**
		* @return int whether we want or not to load an initial set of weights
		*/		
		int getToLoadWeights();
		/**
		* @return string path to the file containing the weights to be loaded
		*/
		std::string getPathToWeightsFiles();
		/**
		* @return int learningLength number of frames to be learned in total, e.g. 50,000,000 (DQN).
		*/
		int getLearningLength();
        int getToSaveCheckPoint();
        std::string getCheckPointName();
};
