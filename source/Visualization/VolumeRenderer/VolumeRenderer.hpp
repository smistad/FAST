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
        
		//void resizeEvent(QResizeEvent* event);
		void mouseEvents();

		void motion(int , int);
		BoundingBox getBoundingBox();

		void setIncludeGeometry(bool p);
		void addGeometryColorTexture(GLuint geoColorTex);
		void addGeometryDepthTexture(GLuint geoDepthTex);
        
		void resize(GLuint, GLuint);
		void setProjectionParameters(float fov, float aspect, float nearPlane, float farPlane);
		void setUserTransform(int volumeIndex, const float userTransform[16]);
    private:
        VolumeRenderer();
        void execute();
        void draw();
		void turnOffTransformations();

		GLuint mHeight;
		GLuint mWidth;

		float topOfViewPlane, rightOfViewPlane;
		float zNear, zFar;
		float projectionMatrix10, projectionMatrix14;

		cl::Image2DGL mImageGLGeoColor;
		cl::Image2DGL mImageGLGeoDepth;
		
        OpenCLDevice::pointer mDevice;
        
		bool includeGeometry;

        bool mOutputIsCreated;
		bool mDoTransformations;
        //unsigned int windowWidth, windowHeight;

        cl::Program program;
		cl::Context clContext;

       // unsigned int mWidth, mHeight;


		GLuint pbo;
		cl::BufferGL pbo_cl;

		cl::Image2D d_transferFuncArray[maxNumberOfVolumes];
		cl::Image2D d_opacityFuncArray[maxNumberOfVolumes];

		cl::Buffer d_invViewMatrix;
		cl::Buffer d_invProjectionModelView;
		
		float invProjectionModelView[16];

		cl::Kernel renderKernel;

		bool mInputIsModified;

		int ox, oy;

		//GLfloat viewRotation[3];
		//GLfloat viewTranslation[3];
		//GLfloat viewRotationPoint[3];
		bool gluInvertMatrix(const float m[16], float invOut[16]);

		unsigned int numberOfVolumes;

		std::vector<ImageData::pointer> mInputs;
		std::vector<Image::pointer> inputs;


		float* transferFunc;
		float* opacityFunc;

		bool doUserTransforms[maxNumberOfVolumes];
		float mUserTransforms[maxNumberOfVolumes*16];
		float mUserTransform0[16];
		float mUserTransform1[16];
		float mUserTransform2[16];
		float mUserTransform3[16];
		float mUserTransform4[16];

		float invViewMatrix0[16];
		float invViewMatrix1[16];
		float invViewMatrix2[16];
		float invViewMatrix3[16];
		float invViewMatrix4[16];
		float invViewMatrix[16];

};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


