#include <tuple>
#include <vector>

/* These are just to make the code less verbose. */
typedef unsigned char u_char;
typedef std::vector<std::tuple<int,int> >::iterator t_iter;

int getNumberOfFeatures(int numRows, int numColumns, int numColors);

void getBROSTFeatures(std::vector<int>* features, const u_char *screen, int screenHeight,
    int screenWidth, int numRows, int numColumns, int numColors);