#include "SliceRenderer.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
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

#ifndef GL_RGBA32F
#define GL_RGBA32F 34838
#endif

using namespace fast;

#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
static Display * mXDisplay;
#endif
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

#if defined(__APPLE__) || defined(__MACOSX)
    // Returns 0 on success
    bool success = CGLSetCurrentContext((CGLContextObj)mDevice->getGLContext()) == 0;
#else
#if _WIN32
    bool success = wglMakeCurrent(wglGetCurrentDC(), (HGLRC)mDevice->getGLContext());
#else
    bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)mDevice->getGLContext());
#endif
#endif
    if(!success)
        throw Exception("failed to switch to window");

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, clImage->getImageInfo<CL_IMAGE_WIDTH>(), clImage->getImageInfo<CL_IMAGE_HEIGHT>(), 0, GL_RGBA, GL_FLOAT, 0);
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

    cl::Kernel kernel(mProgram, "renderToTexture");
    kernel.setArg(0, *clImage);
    kernel.setArg(1, mImageGL);
    kernel.setArg(2, (int)(clImage->getImageInfo<CL_IMAGE_DEPTH>()/2));
    kernel.setArg(3, level);
    kernel.setArg(4, window);
    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(clImage->getImageInfo<CL_IMAGE_WIDTH>(), clImage->getImageInfo<CL_IMAGE_HEIGHT>()),
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
    /*
    if(image->getNrOfDimensions() != 3)
        throw Exception("The SliceRenderer only supports 3D images");
        */
}

SliceRenderer::SliceRenderer() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());
    int i = mDevice->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Visualization/SliceRenderer.cl");
    mProgram = mDevice->getProgram(i);
    mTextureIsCreated = false;
    mIsModified = true;
#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
    // Open the display here to avoid getting maximum number of clients error
    mXDisplay = XOpenDisplay(NULL);
#endif
#endif
}

void SliceRenderer::draw() {
    std::cout << "calling draw()" << std::endl;

    if(!mTextureIsCreated)
        return;

#if defined(__APPLE__) || defined(__MACOSX)
    // Returns 0 on success
    bool success = CGLSetCurrentContext((CGLContextObj)mDevice->getGLContext()) == 0;
#else
#if _WIN32
    bool success = wglMakeCurrent(wglGetCurrentDC(), (HGLRC)mDevice->getGLContext());
#else
    bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)mDevice->getGLContext());
#endif
#endif
    if(!success)
        throw Exception("failed to switch to window");

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
