#include "SliceRenderer.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
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
#endif
#endif

using namespace fast;

#ifndef GL_RGBA32F // this is missing on windows and mac for some reason
#define GL_RGBA32F 0x8814
#endif


void SliceRenderer::execute() {
    if(!mInput.isValid())
        throw Exception("No input was given to SliceRenderer");

    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    if(input->getDimensions() != 3)
        throw Exception("The SliceRenderer only supports 3D images");

    // Determine level and window
    float window = mWindow;
    float level = mLevel;
    // If mWindow/mLevel is equal to -1 use default level/window values
    if(window == -1) {
        window = getDefaultIntensityWindow(input->getDataType());
    }
    if(level == -1) {
        level = getDefaultIntensityLevel(input->getDataType());
    }

    setOpenGLContext(mDevice->getGLContext());

    // Determine slice nr and width and height of the texture to render to
    unsigned int sliceNr;
    if(mSliceNr == -1) {
        switch(mSlicePlane) {
        case PLANE_X:
            sliceNr = input->getWidth()/2;
            break;
        case PLANE_Y:
            sliceNr = input->getHeight()/2;
            break;
        case PLANE_Z:
            sliceNr = input->getDepth()/2;
            break;
        }
    } else if(mSliceNr >= 0) {
        // Check that mSliceNr is valid
        sliceNr = mSliceNr;
        switch(mSlicePlane) {
        case PLANE_X:
            if(sliceNr >= input->getWidth())
                sliceNr = input->getWidth()-1;
            break;
        case PLANE_Y:
            if(sliceNr >= input->getHeight())
                sliceNr = input->getHeight()-1;
            break;
        case PLANE_Z:
            if(sliceNr >= input->getDepth())
                sliceNr = input->getDepth()-1;
            break;
        }
    } else {
        throw Exception("Slice to render was below 0 in SliceRenderer");
    }
    unsigned int slicePlaneNr;
    switch(mSlicePlane) {
        case PLANE_X:
            slicePlaneNr = 0;
            mWidth = input->getDepth();
            mHeight = input->getHeight();
            break;
        case PLANE_Y:
            slicePlaneNr = 1;
            mWidth = input->getWidth();
            mHeight = input->getDepth();
            break;
        case PLANE_Z:
            slicePlaneNr = 2;
            mWidth = input->getWidth();
            mHeight = input->getHeight();
            break;
    }

    glViewport(0,0,mWidth*mScale,mHeight*mScale);

    OpenCLImageAccess3D access = input->getOpenCLImageAccess3D(ACCESS_READ, mDevice);
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

    recompileOpenCLCode(input);
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
    addParent(mInput);
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
    int i = mDevice->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Visualization/SliceRenderer/SliceRenderer.cl", buildOptions);
    mKernel = cl::Kernel(mDevice->getProgram(i), "renderToTexture");
    mTypeCLCodeCompiledFor = input->getDataType();
}


SliceRenderer::SliceRenderer() : Renderer() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());
    mTextureIsCreated = false;
    mIsModified = true;
    mSlicePlane = PLANE_Y;
    mSliceNr = -1;
    mScale = 1.0;
}

void SliceRenderer::draw() {
    if(!mTextureIsCreated)
        return;

    setOpenGLContext(mDevice->getGLContext());

    glBindTexture(GL_TEXTURE_2D, mTexture);

    glBegin(GL_QUADS);
        glTexCoord2i(0, 1);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glTexCoord2i(1, 1);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2i(1, 0);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glTexCoord2i(0, 0);
        glVertex3f(-1.0f,-1.0f, 0.0f);
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

void SliceRenderer::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    case Qt::Key_X:
        mSlicePlane = PLANE_X;
        mIsModified = true;
        break;
    case Qt::Key_Y:
        mSlicePlane = PLANE_Y;
        mIsModified = true;
        break;
    case Qt::Key_Z:
        mSlicePlane = PLANE_Z;
        mIsModified = true;
        break;
    case Qt::Key_Plus:
        mScale = mScale*1.5;
        glViewport(0,0,mWidth*mScale,mHeight*mScale);
        break;
    case Qt::Key_Minus:
        mScale = mScale/1.5;
        glViewport(0,0,mWidth*mScale,mHeight*mScale);
        break;
    }
}
