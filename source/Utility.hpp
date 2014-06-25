#ifndef UTILITY_HPP_
#define UTILITY_HPP_

// This file contains a set of utility functions

// Undefine windows crap
#undef min
#undef max

namespace fast {

double log2(double n);
double round(double n);
int pow(int a, int b);

template<class T>
T min(T a, T b) {
    return a < b ? a : b;
}

template<class T>
T max(T a, T b) {
    return a > b ? a : b;
}

} // end namespace fast


#endif /* UTILITY_HPP_ */
