#ifndef OPACITYTRANSFERFUNCTION_HPP_
#define OPACITYTRANSFERFUNCTION_HPP_

#include "FAST/Object.hpp"
#include <vector>

namespace fast {

class FAST_EXPORT  AlphaPoint
{
public:
	AlphaPoint(double x, double a){
		X=x;
		A=a;
	}
  double X;
  double A;
};
class FAST_EXPORT  OpacityTransferFunction : public Object {
    FAST_OBJECT(OpacityTransferFunction)
    public:
        void addAlphaPoint(double x, double a);
		void addAlphaSegment(double x1, double a1, double x2, double a2);
		double getXMax();
		double getXMin();
		//void setThreshold(float threshold);
        //void keyPressEvent(QKeyEvent* event);
        //void mouseMoveEvent(QMouseEvent* event, View* view);
        //void resizeEvent(QResizeEvent* event);
		//void motion(int , int);
		std::vector<fast::AlphaPoint> v;
    private:
		double XMax;
		double XMin;
        OpacityTransferFunction();
        
};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


