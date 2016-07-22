To compile the C++ library:

>> g++ -std=c++11 -shared -Wl,-install_name,BPROSLibrary.so -o BPROSLibrary.so -fPIC BPROSLibrary.cpp
>> g++ -std=c++11 -shared -Wl,-install_name,BPROSTLibrary.so -o BPROSTLibrary.so -fPIC BPROSTLibrary.cpp
