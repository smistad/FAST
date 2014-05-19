#ifndef VOLUMERENDERER_HPP_
#define VOLUMERENDERER_HPP_

#include "Renderer.hpp"
#include "ImageData.hpp"

namespace fast {

class VolumeRenderer : public Renderer {
    FAST_OBJECT(VolumeRenderer)
    public:
        void setInput(ImageData::pointer image);
        //void setThreshold(float threshold);
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event, View* view);
        void resizeEvent(QResizeEvent* event);
		void motion(int , int);
    private:
        VolumeRenderer();
        void execute();
        void draw();


        OpenCLDevice::pointer mDevice;
        ImageData::pointer mInput;

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
		cl::Image3D* d_volumeArray;
		bool updated;

		int ox, oy;

		GLfloat viewRotation[3];
		GLfloat viewTranslation[3];

		cl::Image3D xx;
};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


