
#include "OpacityTransferFunction.hpp"
#include "FAST/Data/Image.hpp"
#include <limits.h>


namespace fast {

OpacityTransferFunction::OpacityTransferFunction(){
#undef min
#undef max
	XMax = std::numeric_limits<double>::min();
	XMin = std::numeric_limits<double>::max();
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
}
void OpacityTransferFunction::addAlphaPoint(double x, double a) {

	v.push_back(AlphaPoint(x, a));
	XMin = min(x, XMin);
	XMax = max(x, XMax);
}
void OpacityTransferFunction::addAlphaSegment(double x1, double a1, double x2, double a2){
	
}

double OpacityTransferFunction::getXMax(){
	
	return XMax;
}

double OpacityTransferFunction::getXMin(){
	
	return XMin;
}

} // namespace fast

