#ifndef VOLUMERENDERER_HPP_
#define VOLUMERENDERER_HPP_

#define maxNumberOfVolumes 5

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"

namespace fast {

class FAST_EXPORT  VolumeRenderer : public Renderer {
    FAST_OBJECT(VolumeRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
		void setColorTransferFunction(int volumeIndex, ColorTransferFunction::pointer ctf);
		void setOpacityTransferFunction(int volumeIndex, OpacityTransferFunction::pointer otf);
		void setModelViewMatrix(GLfloat mView[16]);

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

		void turnOffTransformations();

    private:
        VolumeRenderer();
        void execute();
        void draw();
		

		GLuint mHeight;
		GLuint mWidth;

		float topOfViewPlane, rightOfViewPlane;
		float zNear, zFar;
		float projectionMatrix10, projectionMatrix14;
#if defined(CL_VERSION_1_2)
		cl::ImageGL mImageGLGeoColor;
		cl::ImageGL mImageGLGeoDepth;
#else
		cl::Image2DGL mImageGLGeoColor;
		cl::Image2DGL mImageGLGeoDepth;
#endif
		
        OpenCLDevice::pointer mDevice;
        
		bool includeGeometry;

        bool mOutputIsCreated;
		bool mDoTransformations;
        //unsigned int windowWidth, windowHeight;

		std::mutex mMutex;

        cl::Program program;
		cl::Context clContext;

       // unsigned int mWidth, mHeight;


		GLuint pbo;
		cl::BufferGL pbo_cl;

		cl::Image2D d_transferFuncArray[maxNumberOfVolumes];
		cl::Image2D d_opacityFuncArray[maxNumberOfVolumes];

		cl::Buffer d_invViewMatrices;
		cl::Buffer d_boxMaxs;
		
		

		cl::Kernel renderKernel;

		bool mInputIsModified;

		int ox, oy;

		//GLfloat viewRotation[3];
		//GLfloat viewTranslation[3];
		//GLfloat viewRotationPoint[3];
		bool gluInvertMatrix(const float m[16], float invOut[16]);

		unsigned int numberOfVolumes;
		
		
		//Image::pointer 
		Image::pointer mI;
		std::vector<Image::pointer> mInputs;
		std::vector<Image::pointer> inputs;
		
		GLfloat modelView[16];

		float* transferFunc;
		float* opacityFunc;

		bool doUserTransforms[maxNumberOfVolumes];
		float mUserTransforms[maxNumberOfVolumes * 16];
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
		float invViewMatrices[maxNumberOfVolumes * 16];
		
		float boxMaxs[maxNumberOfVolumes * 3];

		float opacityFuncDefs[maxNumberOfVolumes];
		float opacityFuncMins[maxNumberOfVolumes];
		float colorFuncDefs[maxNumberOfVolumes];
		float colorFuncMins[maxNumberOfVolumes];

		cl::Buffer d_opacityFuncDefs;
		cl::Buffer d_opacityFuncMins;
		cl::Buffer d_colorFuncDefs;
		cl::Buffer d_colorFuncMins;

};

} // namespace fast


#endif /* VOLUMERENDERER_HPP_ */


