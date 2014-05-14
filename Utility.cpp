#include "Utility.hpp"
#include <cmath>

namespace fast {

double log2(double n) {
    return log( n ) / log( 2.0 );
}

double round(double n) {
	return (n - floor(n) > 0.5) ? ceil(n) : floor(n);
}

int pow(int a, int b) {
    return (int)std::pow((double)a, (double)b);
}

} // end namespace fast
