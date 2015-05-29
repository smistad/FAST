#ifndef COLORTRANSFERFUNCTION_HPP_
#define COLORTRANSFERFUNCTION_HPP_

#include "FAST/Object.hpp"
#include <vector>

namespace fast {

class RGBPoint
{
public:
	RGBPoint(double x, double r, double g, double b){
		X=x;
		R=r;
		G=g;
		B=b;
	}
  double X;
  double R;
  double G;
  double B;
};
class ColorTransferFunction : public Object {
    FAST_OBJECT(ColorTransferFunction)
    public:

        void addRGBPoint(double x, double r, double g, double b);
		void addRGBSegment(double x1, double r1, double g1, double b1, double x2, double r2, double g2, double b2);
		double getXMax();
		double getXMin();
		//void setThreshold(float threshold);
        //void keyPressEvent(QKeyEvent* event);
        //void mouseMoveEvent(QMouseEvent* event, View* view);
        //void resizeEvent(QResizeEvent* event);
		//void motion(int , int);
		std::vector<fast::RGBPoint> v;
    private:
		double XMax;
		double XMin;
        ColorTransferFunction();
        //void execute();
        //void draw();

		/*
        OpenCLDevice::pointer mDevice;
        

        bool mOutputIsCreated;

        //unsigned int windowWidth, windowHeight;

        cl::Program program;


        unsigned int mWidth, mHeight;


		GLuint pbo;
		cl::BufferGL pbo_cl;
		cl::Sampler volumeSamplerLinear;
		cl::Image2D d_transferFuncArray;
		cl::Sampler transferFuncSampler;
		cl::Buffer d_invViewMatrix;
		float invViewMatrix[12];

		cl::Kernel renderKernel;

		bool updated;

		int ox, oy;

		GLfloat viewRotation[3];
		GLfloat viewTranslation[3];

		
		unsigned int numberOfVolumes;

		std::vector<ImageData::pointer> mInputs;
		std::vector<Image::pointer> inputs;
		static const unsigned int maxNumberOfVolumes=5;

		*/

};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


