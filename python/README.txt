To compile the C++ library:

>> g++ -std=c++11 -shared -Wl,-install_name,visual_features.so -o visual_features.so -fPIC visual_features.cpp
