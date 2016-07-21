To compile the C++ library:

>> g++ -std=c++11 -shared -Wl,-install_name,BPROSLibrary.so -o BPROSLibrary.so -fPIC BPROSLibrary.cpp
