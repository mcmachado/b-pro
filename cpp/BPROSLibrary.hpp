#include <vector>

int getNumberOfFeatures(int numRows, int numColumns, int numColors);

void getBROSFeatures(std::vector<int>* features, const u_char *screen, int screenHeight,
    int screenWidth, int numRows, int numColumns, int numColors);