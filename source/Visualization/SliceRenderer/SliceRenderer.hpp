#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "Renderer.hpp"
#include "Image.hpp"

namespace fast {

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

class SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void setInputConnection(ProcessObjectPort port);
        void setSliceToRender(unsigned int sliceNr);
        void setSlicePlane(PlaneType plane);
		void setSlicePlaneNormal(float x, float y, float z);
		void setSlicePlaneNormal(Vector3f slicePlaneNormal);
		void setSlicePlaneOrigin(float x, float y, float z);
		void setSlicePlaneOrigin(Vector3f slicePlaneOrigin);
        BoundingBox getBoundingBox();
        void turnOffTransformations();
    private:
        SliceRenderer();
        void execute();
        void draw();
        void recompileOpenCLCode(Image::pointer input);

        Image::pointer mImageToRender;
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        GLuint mTexture;
        bool mTextureIsCreated;

        cl::Kernel mKernel;
        DataType mTypeCLCodeCompiledFor;

        PlaneType mSlicePlane;

        float mScale;
        unsigned int mWidth;
        unsigned int mHeight;

		unsigned int minX, maxX;
		unsigned int minY, maxY;
		unsigned int minZ, maxZ;

		Vector3f corners[4];

		Vector3f planeNormal;
		Vector3f planeOrigin;

		float planeD;

        bool mDoTransformations;

        boost::mutex mMutex;
};

}




#endif /* SLICERENDERER_HPP_ */
