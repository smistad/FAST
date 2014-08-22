#ifndef VOLUMERENDERER_HPP_
#define VOLUMERENDERER_HPP_

#define maxNumberOfVolumes 5

#include "Renderer.hpp"
#include "Image.hpp"
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"

namespace fast {

class VolumeRenderer : public Renderer {
    FAST_OBJECT(VolumeRenderer)
    public:
        void addInput(ImageData::pointer image);
		void setColorTransferFunction(int volumeIndex, ColorTransferFunction::pointer ctf);
		void setOpacityTransferFunction(int volumeIndex, OpacityTransferFunction::pointer otf);
        //void setThreshold(float threshold);
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event, View* view);
        void resizeEvent(QResizeEvent* event);
		void motion(int , int);
        BoundingBox getBoundingBox() {};
    private:
        VolumeRenderer();
        void execute();
        void draw();


        OpenCLDevice::pointer mDevice;
        

        bool mOutputIsCreated;

        //unsigned int windowWidth, windowHeight;

        cl::Program program;
		cl::Context clContext;

        unsigned int mWidth, mHeight;


		GLuint pbo;
		cl::BufferGL pbo_cl;
		cl::Sampler volumeSamplerLinear;
		cl::Image2D d_transferFuncArray[maxNumberOfVolumes];
		cl::Image2D d_opacityFuncArray[maxNumberOfVolumes];
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

		

		float* transferFunc;
		float* opacityFunc;
};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


