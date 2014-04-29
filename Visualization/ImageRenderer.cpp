#include "ImageRenderer.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include <GL/glx.h>
#include "Image.hpp"
#include "DynamicImage.hpp"


using namespace fast;

void ImageRenderer::execute() {
    std::cout << "calling execute() on ImageRenderer" << std::endl;
    if(!mInput.isValid())
        throw Exception("No input was given to ImageRenderer");

    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
        std::cout << "processing a new frame" << std::endl;
    } else {
        input = mInput;
    }

    bool success = glXMakeCurrent(XOpenDisplay(0),glXGetCurrentDrawable(),(GLXContext)mDevice->getGLContext());
    if(!success)
        throw Exception("failed to switch to window");

    OpenCLImageAccess2D access = input->getOpenCLImageAccess(ACCESS_READ, mDevice);
    cl::Image2D* clImage = access.get();

    // Create OpenGL texture
    glEnable(GL_TEXTURE_2D);
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

void ImageRenderer::setInput(ImageData::pointer image) {
    mInput = image;
    addParent(mInput);
    mIsModified = true;
    /*
    if(image->getNrOfComponents() != 2)
        throw Exception("The ImageRenderer only supports 2D images");
    */
}

ImageRenderer::ImageRenderer() {
    mDevice = boost::static_pointer_cast<OpenCLDevice>(DeviceManager::getInstance().getDefaultVisualizationDevice());
    int i = mDevice->createProgramFromSource(std::string(FAST_ROOT_DIR) + "/Visualization/ImageRenderer.cl");
    mProgram = mDevice->getProgram(i);
    mTextureIsCreated = false;
    mIsModified = true;
}

void ImageRenderer::draw() {
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
    bool success = glXMakeCurrent(XOpenDisplay(0),glXGetCurrentDrawable(),(GLXContext)mDevice->getGLContext());
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
