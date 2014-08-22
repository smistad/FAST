#include "SliceRenderer.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "Image.hpp"
#include "SceneGraph.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#include <GL/glu.h>
#endif
#endif

using namespace fast;

#ifndef GL_RGBA32F // this is missing on windows and mac for some reason
#define GL_RGBA32F 0x8814
#endif


void SliceRenderer::execute() {
    if(!mInput.isValid())
        throw Exception("No input was given to SliceRenderer");

    if(mInput->isDynamicData()) {
        mImageToRender = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        mImageToRender = mInput;
    }

    if(mImageToRender->getDimensions() != 3)
        throw Exception("The SliceRenderer only supports 3D images");

    // Determine level and window
    float window = mWindow;
    float level = mLevel;
    // If mWindow/mLevel is equal to -1 use default level/window values
    if(window == -1) {
        window = getDefaultIntensityWindow(mImageToRender->getDataType());
    }
    if(level == -1) {
        level = getDefaultIntensityLevel(mImageToRender->getDataType());
    }

    setOpenGLContext(mDevice->getGLContext());

    // Determine slice nr and width and height of the texture to render to
    unsigned int sliceNr;
    if(mSliceNr == -1) {
        switch(mSlicePlane) {
        case PLANE_X:
            sliceNr = mImageToRender->getWidth()/2;
            break;
        case PLANE_Y:
            sliceNr = mImageToRender->getHeight()/2;
            break;
        case PLANE_Z:
            sliceNr = mImageToRender->getDepth()/2;
            break;
        }
    } else if(mSliceNr >= 0) {
        // Check that mSliceNr is valid
        sliceNr = mSliceNr;
        switch(mSlicePlane) {
        case PLANE_X:
            if(sliceNr >= mImageToRender->getWidth())
                sliceNr = mImageToRender->getWidth()-1;
            break;
        case PLANE_Y:
            if(sliceNr >= mImageToRender->getHeight())
                sliceNr = mImageToRender->getHeight()-1;
            break;
        case PLANE_Z:
            if(sliceNr >= mImageToRender->getDepth())
                sliceNr = mImageToRender->getDepth()-1;
            break;
        }
    } else {
        throw Exception("Slice to render was below 0 in SliceRenderer");
    }
    unsigned int slicePlaneNr;
    switch(mSlicePlane) {
        case PLANE_X:
            slicePlaneNr = 0;
            mWidth = mImageToRender->getHeight();
            mHeight = mImageToRender->getDepth();
            break;
        case PLANE_Y:
            slicePlaneNr = 1;
            mWidth = mImageToRender->getWidth();
            mHeight = mImageToRender->getDepth();
            break;
        case PLANE_Z:
            slicePlaneNr = 2;
            mWidth = mImageToRender->getWidth();
            mHeight = mImageToRender->getHeight();
            break;
    }
    mSliceNr = sliceNr;

    OpenCLImageAccess3D access = mImageToRender->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
    cl::Image3D* clImage = access.get();

    glEnable(GL_TEXTURE_2D);
    if(mTextureIsCreated) {
        // Delete old texture
        glDeleteTextures(1, &mTexture);
    }

    // Create OpenGL texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFinish();

    // Create CL-GL image
#if defined(CL_VERSION_1_2)
    mImageGL = cl::ImageGL(
            mDevice->getContext(),
            CL_MEM_READ_WRITE,
            GL_TEXTURE_2D,
            0,
            mTexture
    );
#else
    mImageGL = cl::Image2DGL(
            mDevice->getContext(),
            CL_MEM_READ_WRITE,
            GL_TEXTURE_2D,
            0,
            mTexture
    );
#endif

    // Run kernel to fill the texture
    cl::CommandQueue queue = mDevice->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(mImageGL);
    queue.enqueueAcquireGLObjects(&v);

    recompileOpenCLCode(mImageToRender);
    mKernel.setArg(0, *clImage);
    mKernel.setArg(1, mImageGL);
    mKernel.setArg(2, sliceNr);
    mKernel.setArg(3, level);
    mKernel.setArg(4, window);
    mKernel.setArg(5, slicePlaneNr);
    queue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            cl::NDRange(mWidth, mHeight),
            cl::NullRange
    );

    queue.enqueueReleaseGLObjects(&v);
    queue.finish();

    mTextureIsCreated = true;
}

void SliceRenderer::setInput(ImageData::pointer image) {
    mInput = image;
    setParent(mInput);
    mIsModified = true;
}

void SliceRenderer::recompileOpenCLCode(Image::pointer input) {
    // Check if code has to be recompiled
    bool recompile = false;
    if(!mTextureIsCreated) {
        recompile = true;
    } else {
        if(mTypeCLCodeCompiledFor != input->getDataType())
            recompile = true;
    }
    if(!recompile)
        return;
    std::string buildOptions = "";
    if(input->getDataType() == TYPE_FLOAT) {
        buildOptions = "-DTYPE_FLOAT";
    } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
        buildOptions = "-DTYPE_INT";
    } else {
        buildOptions = "-DTYPE_UINT";
    }
    int i = mDevice->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/SliceRenderer/SliceRenderer.cl", buildOptions);
    mKernel = cl::Kernel(mDevice->getProgram(i), "renderToTexture");
    mTypeCLCodeCompiledFor = input->getDataType();
}


SliceRenderer::SliceRenderer() : Renderer() {
    mDevice = DeviceManager::getInstance().getDefaultVisualizationDevice();
    mTextureIsCreated = false;
    mIsModified = true;
    mSlicePlane = PLANE_Z;
    mSliceNr = -1;
    mScale = 1.0;
    mDoTransformations = true;
}

void SliceRenderer::draw() {
    if(!mTextureIsCreated)
        return;

    //setOpenGLContext(mDevice->getGLContext());

    if(mDoTransformations) {
        SceneGraph& graph = SceneGraph::getInstance();
        SceneGraphNode::pointer node = graph.getDataNode(mImageToRender);
        LinearTransformation transform = graph.getLinearTransformationFromNode(node);

        float matrix[16] = {
                transform(0,0), transform(1,0), transform(2,0), transform(3,0),
                transform(0,1), transform(1,1), transform(2,1), transform(3,1),
                transform(0,2), transform(1,2), transform(2,2), transform(3,2),
                transform(0,3), transform(1,3), transform(2,3), transform(3,3)
        };

        glMultMatrixf(matrix);
    }

    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Draw slice in voxel coordinates
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    switch(mSlicePlane) {
    case PLANE_Z:
        glTexCoord2i(0, 1);
        glVertex3f(0, mHeight, mSliceNr);
        glTexCoord2i(1, 1);
        glVertex3f(mWidth, mHeight, mSliceNr);
        glTexCoord2i(1, 0);
        glVertex3f( mWidth,0, mSliceNr);
        glTexCoord2i(0, 0);
        glVertex3f(0,0, mSliceNr);
        break;
    case PLANE_Y:
        glTexCoord2i(0, 1);
        glVertex3f(0, mSliceNr, mHeight);
        glTexCoord2i(1, 1);
        glVertex3f(mWidth, mSliceNr, mHeight);
        glTexCoord2i(1, 0);
        glVertex3f( mWidth,mSliceNr, 0 );
        glTexCoord2i(0, 0);
        glVertex3f(0,mSliceNr,0 );
        break;
    case PLANE_X:
        glTexCoord2i(0, 1);
        glVertex3f(mSliceNr, 0 , mHeight);
        glTexCoord2i(1, 1);
        glVertex3f(mSliceNr, mWidth, mHeight);
        glTexCoord2i(1, 0);
        glVertex3f( mSliceNr, mWidth, 0 );
        glTexCoord2i(0, 0);
        glVertex3f(mSliceNr,0,0 );
    break;
    }
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SliceRenderer::setSliceToRender(unsigned int sliceNr) {
    mSliceNr = sliceNr;
    mIsModified = true;
}

void SliceRenderer::setSlicePlane(PlaneType plane) {
    mSlicePlane = plane;
    mIsModified = true;
}

BoundingBox SliceRenderer::getBoundingBox() {

    BoundingBox inputBoundingBox = mImageToRender->getBoundingBox();
    Vector<Float3, 8> corners = inputBoundingBox.getCorners();
    // Shrink bounding box so that it covers the slice and not the entire data
    switch(mSlicePlane) {
        case PLANE_X:
            for(uint i = 0; i < 8; i++)
                corners[i][0] = mSliceNr;
            break;
        case PLANE_Y:
            for(uint i = 0; i < 8; i++)
                corners[i][1] = mSliceNr;
            break;
        case PLANE_Z:
            for(uint i = 0; i < 8; i++)
                corners[i][2] = mSliceNr;
            break;
    }
    BoundingBox shrinkedBox(corners);
    if(mDoTransformations) {
        SceneGraph& graph = SceneGraph::getInstance();
        SceneGraphNode::pointer node;
        node = graph.getDataNode(mImageToRender);
        LinearTransformation transform = graph.getLinearTransformationFromNode(node);
        BoundingBox transformedBoundingBox = shrinkedBox.getTransformedBoundingBox(transform);
        return transformedBoundingBox;
    } else {
        return shrinkedBox;
    }
}

void SliceRenderer::turnOffTransformations() {
    mDoTransformations = false;
}
