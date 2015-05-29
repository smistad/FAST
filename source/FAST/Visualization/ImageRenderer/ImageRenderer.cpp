#include "ImageRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "HelperFunctions.hpp"
#include "FAST/SceneGraph.hpp"
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

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Image::pointer input = getStaticInputData<Image>(inputNr);

        mImagesToRender[inputNr] = input;
    }
}

void ImageRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Image>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}


ImageRenderer::ImageRenderer() : Renderer() {
    createInputPort<Image>(0, false);
    mIsModified = false;
}

void ImageRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        Image::pointer input = it->second;
        uint inputNr = it->first;

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input)
            continue; // If it has already been created, skip it

        // If it has not been created, create the texture

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
        mImageUsed[inputNr] = input;

        // Create CL-GL image
        #if defined(CL_VERSION_1_2)
            cl::ImageGL mImageGL = cl::ImageGL(
                    device->getContext(),
                    CL_MEM_READ_WRITE,
                    GL_TEXTURE_2D,
                    0,
                    textureID
            );
        #else
            cl::Image2DGL mImageGL = cl::Image2DGL(
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


    // This is the actual rendering
    for(it = mImageUsed.begin(); it != mImageUsed.end(); it++) {
        glPushMatrix();

        AffineTransformation transform = SceneGraph::getAffineTransformationFromData(it->second);
        transform.scale(it->second->getSpacing()); // Apply image spacing
        glMultMatrixf(transform.data());

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

void ImageRenderer::draw2D(cl::BufferGL PBO, uint width, uint height, Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform, float PBOspacing, Vector2f translation) {
    boost::lock_guard<boost::mutex> lock(mMutex);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    v.push_back(PBO);
    queue.enqueueAcquireGLObjects(&v);

    // Create an aux PBO
    cl::Buffer PBO2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(float)*width*height*4
    );

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        Image::pointer input = it->second;
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

        int i = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "/Visualization/ImageRenderer/ImageRenderer2D.cl");

        if(input->getDimensions() == 2) {
            cl::Kernel kernel(device->getProgram(i), "render2Dimage");
            // Run kernel to fill the texture

            OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access->get();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, input->getSpacing().x());
            kernel.setArg(4, input->getSpacing().y());
            kernel.setArg(5, PBOspacing);
            kernel.setArg(6, level);
            kernel.setArg(7, window);
            kernel.setArg(8, translation.x());
            kernel.setArg(9, translation.y());

            // Run the draw 2D kernel
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        } else {

            // Get transform of the image
            AffineTransformation dataTransform = SceneGraph::getAffineTransformationFromData(input);
            dataTransform.scale(it->second->getSpacing()); // Apply image spacing

            // Transfer transformations
            Eigen::Affine3f transform = dataTransform.inverse()*pixelToViewportTransform;

            cl::Buffer transformBuffer(
                    device->getContext(),
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    16*sizeof(float),
                    transform.data()
            );

             cl::Kernel kernel(device->getProgram(i), "render3Dimage");
            // Run kernel to fill the texture

            OpenCLImageAccess3D::pointer access = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access->get();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, transformBuffer);
            kernel.setArg(4, level);
            kernel.setArg(5, window);

            // Run the draw 3D image kernel
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        }

        // Copy PBO2 to PBO
        queue.enqueueCopyBuffer(PBO2, PBO, 0, 0, sizeof(float)*width*height*4);
    }
    queue.enqueueReleaseGLObjects(&v);
    queue.finish();
}

BoundingBox ImageRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;

    boost::unordered_map<uint, Image::pointer>::iterator it;
    for(it = mImagesToRender.begin(); it != mImagesToRender.end(); it++) {
        BoundingBox transformedBoundingBox;
        transformedBoundingBox = it->second->getTransformedBoundingBox();

        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}
