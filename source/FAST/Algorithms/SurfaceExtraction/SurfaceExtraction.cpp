#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#ifdef FAST_MODULE_VISUALIZATION
#include <QOpenGLFunctions_3_3_Core>
#endif

namespace fast {

void SurfaceExtraction::setThreshold(float threshold) {
    mThreshold = threshold;
    mIsModified = true;
}

inline unsigned int getRequiredHistogramPyramidSize(Image::pointer input) {
    unsigned int largestSize = fast::max(fast::max(input->getWidth(), input->getHeight()), input->getDepth());
    int i = 1;
    while(largestSize > pow(2,i)) {
        i++;
    }
    return (unsigned int)pow(2,i);
}

void SurfaceExtraction::execute() {
    Image::pointer input = getStaticInputData<Image>(0);

    if(input->getDimensions() != 3)
        throw Exception("The SurfaceExtraction object only supports 3D images");

    OpenCLDevice::pointer device = getMainDevice();
#if defined(__APPLE__) || defined(__MACOSX)
    const bool writingTo3DTextures = false;
#else
    const bool writingTo3DTextures = device->isWritingTo3DTexturesSupported();
#endif
    cl::Context clContext = device->getContext();
    const unsigned int SIZE = getRequiredHistogramPyramidSize(input);

    if(mHPSize != SIZE) {
        // Have to recreate the HP
        images.clear();
        buffers.clear();
        std::string programName = "";
        // create new HP (if necessary)
        if(writingTo3DTextures) {
            // Create images for the HistogramPyramid
            int bufferSize = SIZE;
            cl_channel_order order1, order2;
            if(device->isImageFormatSupported(CL_R, CL_UNSIGNED_INT8, CL_MEM_OBJECT_IMAGE3D) && device->isImageFormatSupported(CL_RG, CL_UNSIGNED_INT8, CL_MEM_OBJECT_IMAGE3D)) {
            	order1 = CL_R;
            	order2 = CL_RG;
            } else {
            	order1 = CL_RGBA;
            	order2 = CL_RGBA;
            }

            // Make the two first buffers use INT8
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order2, CL_UNSIGNED_INT8), input->getWidth(), input->getHeight(), input->getDepth()));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order1, CL_UNSIGNED_INT8), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            // And the third, fourth and fifth INT16
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order1, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order1, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order1, CL_UNSIGNED_INT16), bufferSize, bufferSize, bufferSize));
            bufferSize /= 2;
            // The rest will use INT32
            for(int i = 5; i < (log2((float)SIZE)); i ++) {
                if(bufferSize == 1)
                    bufferSize = 2; // Image cant be 1x1x1
                images.push_back(cl::Image3D(clContext, CL_MEM_READ_WRITE, cl::ImageFormat(order1, CL_UNSIGNED_INT32), bufferSize, bufferSize, bufferSize));
                bufferSize /= 2;
            }

            // If writing to 3D textures is not supported we to create buffers to write to
       } else {
            programName = "no_3d_write";
            int bufferSize = SIZE*SIZE*SIZE;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(char)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(char)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(short)*bufferSize));
            bufferSize /= 8;
            for(int i = 5; i < (log2((float)SIZE)); i ++) {
                buffers.push_back(cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(int)*bufferSize));
                bufferSize /= 8;
            }

            cubeIndexesBuffer = cl::Buffer(clContext, CL_MEM_READ_WRITE, sizeof(char)*SIZE*SIZE*SIZE);
        }

        // Compile program
        mHPSize = SIZE;
        char buffer[255];
        sprintf(buffer,"-DSIZE=%d", SIZE);
        std::string buildOptions(buffer);
        if(input->getDataType() == TYPE_FLOAT) {
            buildOptions += " -DTYPE_FLOAT";
        } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
            buildOptions += " -DTYPE_INT";
        } else {
            buildOptions += " -DTYPE_UINT";
        }
#if defined(__APPLE__) || defined(__MACOSX)
        buildOptions += " -DMAC_HACK";
#endif
        program = getOpenCLProgram(device, programName, buildOptions);
    }

    cl::Kernel constructHPLevelKernel(program, "constructHPLevel");
    cl::Kernel classifyCubesKernel(program, "classifyCubes");
    cl::Kernel traverseHPKernel(program, "traverseHP");

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    cl::Image3D* clImage = access->get3DImage();

    // update scalar field
    if(writingTo3DTextures) {
        classifyCubesKernel.setArg(0, images[0]);
        classifyCubesKernel.setArg(1, *clImage);
        classifyCubesKernel.setArg(2, mThreshold);
    } else {
        classifyCubesKernel.setArg(0, buffers[0]);
        classifyCubesKernel.setArg(1, cubeIndexesBuffer);
        classifyCubesKernel.setArg(2, *clImage);
        classifyCubesKernel.setArg(3, mThreshold);
    }
    cl::CommandQueue queue = device->getCommandQueue();
    queue.enqueueNDRangeKernel(
            classifyCubesKernel,
            cl::NullRange,
            cl::NDRange(SIZE, SIZE, SIZE),
            cl::NullRange
    );

    // Construct HP

    if(writingTo3DTextures) {
        // Run base to first level
        constructHPLevelKernel.setArg(0, images[0]);
        constructHPLevelKernel.setArg(1, images[1]);

        queue.enqueueNDRangeKernel(
            constructHPLevelKernel,
            cl::NullRange,
            cl::NDRange(SIZE/2, SIZE/2, SIZE/2),
            cl::NullRange
        );

        int previous = SIZE / 2;
        // Run level 2 to top level
        for(int i = 1; i < log2((float)SIZE)-1; i++) {
            constructHPLevelKernel.setArg(0, images[i]);
            constructHPLevelKernel.setArg(1, images[i+1]);
            previous /= 2;
            queue.enqueueNDRangeKernel(
                constructHPLevelKernel,
                cl::NullRange,
                cl::NDRange(previous, previous, previous),
                cl::NullRange
            );
        }
    } else {
        cl::Kernel constructHPLevelCharCharKernel(program, "constructHPLevelCharChar");
        cl::Kernel constructHPLevelCharShortKernel(program, "constructHPLevelCharShort");
        cl::Kernel constructHPLevelShortShortKernel(program, "constructHPLevelShortShort");
        cl::Kernel constructHPLevelShortIntKernel(program, "constructHPLevelShortInt");

        // Run base to first level
        constructHPLevelCharCharKernel.setArg(0, buffers[0]);
        constructHPLevelCharCharKernel.setArg(1, buffers[1]);

        queue.enqueueNDRangeKernel(
            constructHPLevelCharCharKernel,
            cl::NullRange,
            cl::NDRange(SIZE/2, SIZE/2, SIZE/2),
            cl::NullRange
        );

        int previous = SIZE / 2;

        constructHPLevelCharShortKernel.setArg(0, buffers[1]);
        constructHPLevelCharShortKernel.setArg(1, buffers[2]);

        queue.enqueueNDRangeKernel(
            constructHPLevelCharShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortShortKernel.setArg(0, buffers[2]);
        constructHPLevelShortShortKernel.setArg(1, buffers[3]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortShortKernel.setArg(0, buffers[3]);
        constructHPLevelShortShortKernel.setArg(1, buffers[4]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortShortKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        constructHPLevelShortIntKernel.setArg(0, buffers[4]);
        constructHPLevelShortIntKernel.setArg(1, buffers[5]);

        queue.enqueueNDRangeKernel(
            constructHPLevelShortIntKernel,
            cl::NullRange,
            cl::NDRange(previous/2, previous/2, previous/2),
            cl::NullRange
        );

        previous /= 2;

        // Run level 5 to top level
        for(int i = 5; i < log2((float)SIZE)-1; i++) {
            constructHPLevelKernel.setArg(0, buffers[i]);
            constructHPLevelKernel.setArg(1, buffers[i+1]);
            previous /= 2;
            queue.enqueueNDRangeKernel(
                constructHPLevelKernel,
                cl::NullRange,
                cl::NDRange(previous, previous, previous),
                cl::NullRange
            );
        }
    }

    // Create VBO using the sum
    // Read top of histoPyramid an use this size to allocate VBO below
    unsigned int totalSum = 0;
    if(writingTo3DTextures) {
    	int* sum;
        if(device->isImageFormatSupported(CL_R, CL_UNSIGNED_INT32, CL_MEM_OBJECT_IMAGE3D)) {
            sum = new int[8];
        } else {
        	// A 4 channel texture/image has been used
        	sum = new int[8*4];
        }
        cl::size_t<3> origin = createOrigoRegion();
        cl::size_t<3> region = createRegion(2,2,2);
        queue.enqueueReadImage(images[images.size()-1], CL_TRUE, origin, region, 0, 0, sum);
        if(device->isImageFormatSupported(CL_R, CL_UNSIGNED_INT8, CL_MEM_OBJECT_IMAGE3D)) {
            totalSum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7] ;
        } else {
            totalSum = sum[0] + sum[4] + sum[8] + sum[12] + sum[16] + sum[20] + sum[24] + sum[28] ;
        }
        delete[] sum;
    } else {
        int* sum = new int[8];
        queue.enqueueReadBuffer(buffers[buffers.size()-1], CL_TRUE, 0, sizeof(int)*8, sum);
        totalSum = sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7] ;
        delete[] sum;
    }

    Mesh::pointer output = getStaticOutputData<Mesh>(0);
    SceneGraph::setParentNode(output, input);
    BoundingBox box = input->getBoundingBox();
    // Apply spacing scaling to BB
    AffineTransformation::pointer T = AffineTransformation::New();
    Affine3f transform = T->getTransform();
    transform.scale(input->getSpacing());
    T->setTransform(transform);
    output->create(totalSum);
    output->setBoundingBox(box.getTransformedBoundingBox(T));

    if(totalSum == 0) {
        reportInfo() << "No triangles were extracted. Check isovalue." << Reporter::end();
        return;
    }
    reportInfo() << totalSum << " nr of triangles were extracted with the SurfaceExtraction algorithm." << reportEnd();


    // Traverse HP to create triangles and put them in the VBO
    // Make OpenCL buffer from OpenGL buffer
    unsigned int i = 0;
    if(writingTo3DTextures) {
        traverseHPKernel.setArg(0, *clImage);
        for(i = 0; i < images.size(); i++) {
            traverseHPKernel.setArg(i+1, images[i]);
        }
        i += 1;
    } else {
        traverseHPKernel.setArg(0, *clImage);
        traverseHPKernel.setArg(1, cubeIndexesBuffer);
        for(i = 0; i < buffers.size(); i++) {
            traverseHPKernel.setArg(i+2, buffers[i]);
        }
        i += 2;
    }

    VertexBufferObjectAccess::pointer VBOaccess = output->getVertexBufferObjectAccess(ACCESS_READ_WRITE);
    GLuint* VBO_ID = VBOaccess->get();
    cl::Buffer VBOBuffer;
    std::vector<cl::Memory> v;
    if(DeviceManager::isGLInteropEnabled()) {
        VBOBuffer = cl::BufferGL(device->getContext(), CL_MEM_WRITE_ONLY, *VBO_ID);
        v.push_back(VBOBuffer);
        queue.enqueueAcquireGLObjects(&v);
    } else {
        VBOBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                sizeof(float) * totalSum * 18
        );
    }
    traverseHPKernel.setArg(i, VBOBuffer);
    traverseHPKernel.setArg(i+1, mThreshold);
    traverseHPKernel.setArg(i+2, totalSum);
    traverseHPKernel.setArg(i+3, input->getSpacing().x());
    traverseHPKernel.setArg(i+4, input->getSpacing().y());
    traverseHPKernel.setArg(i+5, input->getSpacing().z());

    // Increase the global_work_size so that it is divideable by 64
    int global_work_size = totalSum + 64 - (totalSum - 64*(totalSum / 64));
    // Run a NDRange kernel over this buffer which traverses back to the base level
    queue.enqueueNDRangeKernel(traverseHPKernel, cl::NullRange, cl::NDRange(global_work_size), cl::NDRange(64));

    if(DeviceManager::isGLInteropEnabled()) {
        queue.enqueueReleaseGLObjects(&v);
        queue.finish();
    } else {
        // Transfer OpenCL buffer data to CPU
#ifdef FAST_MODULE_VISUALIZATION
        float *data = new float[18 * totalSum];
        queue.enqueueReadBuffer(
                VBOBuffer,
                CL_TRUE,
                0,
                sizeof(float) * 18 * totalSum,
                data
        );

        // Transfer CPU data to VBO
        QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
        fun->initializeOpenGLFunctions();
        fun->glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
        fun->glBufferData(GL_ARRAY_BUFFER, totalSum * 18 * sizeof(float), data, GL_STATIC_DRAW);
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();

        delete[] data;
#else
        throw Exception("SurfaceExtraction algorithm is disabled since FAST module visualization is disabled");
#endif
    }

}

SurfaceExtraction::SurfaceExtraction() {
    mThreshold = 0.0f;
    mHPSize = 0;
    createInputPort<Image>(0);
    createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/SurfaceExtraction/SurfaceExtraction.cl");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/SurfaceExtraction/SurfaceExtraction_no_3d_write.cl", "no_3d_write");
}


} // end namespace fast

