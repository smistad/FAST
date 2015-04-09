#include "ImageRenderer.hpp"
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
#endif
#endif
#include <boost/thread/lock_guard.hpp>

using namespace fast;

#ifndef GL_RGBA32F // this is missing on windows and mac for some reason
#define GL_RGBA32F 0x8814 
#endif

void ImageRenderer::execute() {
    boost::lock_guard<boost::mutex> lock(mMutex);
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Image::pointer input = getStaticInputData<Image>(inputNr);

        if(input->getDimensions() != 2)
            throw Exception("The ImageRenderer only supports 2D images");

        mImagesToRender[inputNr] = input;
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

        OpenCLDevice::pointer device = getMainDevice();

        OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
        cl::Image2D* clImage = access->get();

        glEnable(GL_TEXTURE_2D);
        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
        }

        // Create OpenGL texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, clImage->getImageInfo<CL_IMAGE_WIDTH>(), clImage->getImageInfo<CL_IMAGE_HEIGHT>(), 0, GL_RGBA, GL_FLOAT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFinish();

        mTexturesToRender[inputNr] = textureID;

        // Create CL-GL image
    #if defined(CL_VERSION_1_2)
        mImageGL = cl::ImageGL(
                device->getContext(),
                CL_MEM_READ_WRITE,
                GL_TEXTURE_2D,
                0,
                textureID
        );
    #else
        mImageGL = cl::Image2DGL(
                device->getContext(),
                CL_MEM_READ_WRITE,
                GL_TEXTURE_2D,
                0,
                textureID
        );
    #endif


        int i = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/ImageRenderer/ImageRenderer.cl");
        std::string kernelName = "renderToTextureInt";
        if(input->getDataType() == TYPE_FLOAT) {
            kernelName = "renderToTextureFloat";
        } else if(input->getDataType() == TYPE_UINT8 || input->getDataType() == TYPE_UINT16) {
            kernelName = "renderToTextureUint";
        }

        mKernel = cl::Kernel(device->getProgram(i), kernelName.c_str());
        // Run kernel to fill the texture
        cl::CommandQueue queue = device->getCommandQueue();
        std::vector<cl::Memory> v;
        v.push_back(mImageGL);
        queue.enqueueAcquireGLObjects(&v);

        mKernel.setArg(0, *clImage);
        mKernel.setArg(1, mImageGL);
        mKernel.setArg(2, level);
        mKernel.setArg(3, window);
        queue.enqueueNDRangeKernel(
                mKernel,
                cl::NullRange,
                cl::NDRange(clImage->getImageInfo<CL_IMAGE_WIDTH>(), clImage->getImageInfo<CL_IMAGE_HEIGHT>()),
                cl::NullRange
        );

        queue.enqueueReleaseGLObjects(&v);
        queue.finish();
    }
    mTextureIsCreated = true;
}

void ImageRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}


ImageRenderer::ImageRenderer() : Renderer() {
    mTextureIsCreated = false;
    mIsModified = false;
    mDoTransformations = true;
}

void ImageRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);
    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        glPushMatrix();
        if(mDoTransformations) {
            LinearTransformation transform = SceneGraph::getLinearTransformationFromData(it->second);

            glMultMatrixf(transform.getTransform().data());
        }

        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[it->first]);
        uint width = it->second->getWidth();
        uint height = it->second->getHeight();

        glColor3f(1,1,1); // black white texture
        glBegin(GL_QUADS);
            glTexCoord2i(0, 0);
            glVertex3f(0, height, 0.0f);
            glTexCoord2i(1, 0);
            glVertex3f(width, height, 0.0f);
            glTexCoord2i(1, 1);
            glVertex3f(width, 0, 0.0f);
            glTexCoord2i(0, 1);
            glVertex3f(0, 0, 0.0f);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glPopMatrix();
    }

}

void ImageRenderer::turnOffTransformations() {
    mDoTransformations = false;
}

BoundingBox ImageRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        BoundingBox transformedBoundingBox;
        if(mDoTransformations) {
            transformedBoundingBox = it->second->getTransformedBoundingBox();
        } else {
            transformedBoundingBox = it->second->getBoundingBox();
        }

        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}
