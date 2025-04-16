// for now, use a simple algorithm to test if CMake is correctly configured

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

struct DFAConfig {
    int numStates;
    int numSymbols;
    int startState;
    int acceptState;
    int **transitionTable;
};