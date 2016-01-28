#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

class SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        void setSliceToRender(unsigned int sliceNr);
        void setSlicePlane(PlaneType plane);
		void setSlicePlaneNormal(float x, float y, float z);
		void setSlicePlaneNormal(Vector3f slicePlaneNormal);
		void setSlicePlaneOrigin(float x, float y, float z);
		void setSlicePlaneOrigin(Vector3f slicePlaneOrigin);
        BoundingBox getBoundingBox();
    private:
        SliceRenderer();
        void execute();
        void draw();
		void recompileOpenCLCode(boost::unordered_map<uint, Image::pointer> inputs);

        //Image::pointer mImageToRender;
		boost::unordered_map<uint, Image::pointer> mImagesToRender;

		boost::unordered_map<uint, cl::Image3D*> mClImage;

		cl::Buffer d_transformationMatrices[4];
		boost::unordered_map<uint, cl::Buffer> d_imageSizes;

#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        GLuint mTexture;
        bool mTextureIsCreated;

        cl::Kernel mKernel;


        PlaneType mSlicePlane;

        float mScale;
         int mWidth;
         int mHeight;

		 int minX, maxX;
		 int minY, maxY;
		 int minZ, maxZ;

		Vector3f corners[4];

		Vector3f planeNormal;
		Vector3f planeOrigin;

		float planeD;

        boost::mutex mMutex;

		bool mInputIsModified;
};

}




#endif /* SLICERENDERER_HPP_ */
