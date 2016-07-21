/*  Simple example for using the functions that implement B-PROS, B-PROST and Blob-PROST
    features. This implementation is supposed to show how one can use the invoke the code
    provided in C using ctypes. The implemented feature sets were originally introduced in
    the paper below. 

    Yitao Liang, Marlos C. Machado, Erik Talvitie, Michael H. Bowling:
    State of the Art Control of Atari Games Using Shallow Reinforcement Learning. 
    AAMAS 2016: 485-493

    Author: Marlos C. Machado
*/

#include <iostream>
#include <ale_interface.hpp>

#include "timer.hpp"
#include "visual_features.hpp"

#ifdef __USE_SDL
  #include <SDL.h>
#endif

using namespace std;

// global variables that encode the representation parameters
int numRows      = 14;
int numCols      = 16;
int numColors    = 128;
int screenHeight = 210;
int screenWidth  = 160;

int main(int argc, char** argv) {
	// just a simple check first, we really need this rom file ;)
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
        return 1;
    }

    float elapsedTime;
	struct timeval tvBegin, tvEnd, tvDiff;
	vector<int> features;

    // Arcade Learning Environment
    ALEInterface ale;
    ale.setInt("random_seed", 123);
    ale.setInt("frame_skip", 5);

    // load rom provided in the command line
    ale.loadROM(argv[1]);
    // get the vector of legal actions
    ActionVect legal_actions = ale.getLegalActionSet();

    // this is how you get the total number of features:
	cout << "Size of action set: " << getNumberOfFeatures(numRows, numCols, numColors) << endl;

    // we play the game, requesting the screen at each iteration
    int previousFrameCount = 0;
    for (int episode = 0; episode < 5; episode++) {
        float totalReward = 0;
        gettimeofday(&tvBegin, NULL);
        while (!ale.game_over()) {
        	// we now get the current ALE screen and put it in an acceptable format
            unsigned char* screen = ale.getScreen().getArray();
        	// we finally call the function that stores the feature vector inside the object
            features.clear();
            getBROSFeatures(&features, screen, screenHeight,
        		screenWidth, numRows, numCols, numColors);
        	// we randomly select an action in the environment to observe the next state
            Action a = legal_actions[rand() % legal_actions.size()];
            float reward = ale.act(a);
            totalReward += reward;
        }
        // for performance purposes we keep track of frame rate
        gettimeofday(&tvEnd, NULL);
		timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
		elapsedTime = float(tvDiff.tv_sec) + float(tvDiff.tv_usec)/1000000.0;
		float fps = (ale.getFrameNumber() - previousFrameCount)/elapsedTime;
		previousFrameCount = ale.getFrameNumber();
        cout << "Episode " << episode + 1 << " ended with score " << totalReward << " running at " << fps << " fps." <<endl;
        ale.reset_game();
    }

    return 0;
}
