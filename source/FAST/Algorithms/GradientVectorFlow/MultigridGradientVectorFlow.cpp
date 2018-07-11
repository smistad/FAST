#include "MultigridGradientVectorFlow.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"

namespace fast {

cl::Image3D MultigridGradientVectorFlow::initSolutionToZero(Vector3ui size, int imageType, int bufferSize) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D v(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Kernel initToZeroKernel(mProgram, "initFloatBuffer");
        cl::Buffer vBuffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z()
        );
        initToZeroKernel.setArg(0,vBuffer);
        queue.enqueueNDRangeKernel(
                initToZeroKernel,
                cl::NullRange,
                cl::NDRange(size.x()*size.y()*size.z()),
                cl::NullRange
        );
        queue.enqueueCopyBufferToImage(vBuffer,v,0,createOrigoRegion(),createRegion(size.x(), size.y(), size.z()));
    } else {
        cl::Kernel initToZeroKernel(mProgram, "init3DFloat");
        initToZeroKernel.setArg(0,v);
        device->getCommandQueue().enqueueNDRangeKernel(
                initToZeroKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

    }

    return v;
}
void MultigridGradientVectorFlow::gaussSeidelSmoothing(
        cl::Image3D &v,
        cl::Image3D &r,
        cl::Image3D &sqrMag,
        int iterations,
        Vector3ui size,
        float mu,
        float spacing,
        int imageType,
        int bufferSize
        ) {

    if(iterations <= 0)
        return;
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    cl::Kernel gaussSeidelKernel(mProgram, "GVFgaussSeidel");
    cl::Kernel gaussSeidelKernel2(mProgram, "GVFgaussSeidel2");

    cl::Image3D v_2 = cl::Image3D(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
     );

    gaussSeidelKernel.setArg(0, r);
    gaussSeidelKernel.setArg(1, sqrMag);
    gaussSeidelKernel.setArg(2, mu);
    gaussSeidelKernel.setArg(3, spacing);
    gaussSeidelKernel2.setArg(0, r);
    gaussSeidelKernel2.setArg(1, sqrMag);
    gaussSeidelKernel2.setArg(2, mu);
    gaussSeidelKernel2.setArg(3, spacing);

    if(!device->isWritingTo3DTexturesSupported()) {
        cl::size_t<3> offset = createOrigoRegion();
        cl::size_t<3> region = createRegion(size.x(), size.y(), size.z());
        cl::Buffer v_2_buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z());

        for(int i = 0; i < iterations*2; i++) {
             if(i % 2 == 0) {
                 gaussSeidelKernel.setArg(4, v);
                 gaussSeidelKernel.setArg(5, v_2_buffer);
                 queue.enqueueNDRangeKernel(
                    gaussSeidelKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(), size.y(), size.z()),
                    cl::NullRange
                );
                queue.enqueueCopyBufferToImage(v_2_buffer, v_2,0,offset,region);
             } else {
                 gaussSeidelKernel2.setArg(4, v_2);
                 gaussSeidelKernel2.setArg(5, v_2_buffer);
                 queue.enqueueNDRangeKernel(
                    gaussSeidelKernel2,
                    cl::NullRange,
                    cl::NDRange(size.x(), size.y(), size.z()),
                    cl::NullRange
                );
                queue.enqueueCopyBufferToImage(v_2_buffer, v,0,offset,region);
             }
        }
    } else {
         for(int i = 0; i < iterations*2; i++) {
             if(i % 2 == 0) {
                 gaussSeidelKernel.setArg(4, v);
                 gaussSeidelKernel.setArg(5, v_2);
                 queue.enqueueNDRangeKernel(
                    gaussSeidelKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
                );
             } else {
                 gaussSeidelKernel2.setArg(4, v_2);
                 gaussSeidelKernel2.setArg(5, v);
                 queue.enqueueNDRangeKernel(
                    gaussSeidelKernel2,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
                );
             }
        }
    }
}

cl::Image3D MultigridGradientVectorFlow::restrictVolume(
        cl::Image3D &v,
        Vector3ui newSize,
        int imageType,
        int bufferSize
        ) {

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    // Check to see if size is a power of 2 and equal in all dimensions

    cl::Image3D v_2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            newSize.x(),
            newSize.y(),
            newSize.z()
    );

    cl::Kernel restrictKernel(mProgram, "restrictVolume");
    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Buffer v_2_buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*newSize.x()*newSize.y()*newSize.z()
        );
        restrictKernel.setArg(0, v);
        restrictKernel.setArg(1, v_2_buffer);
        queue.enqueueNDRangeKernel(
                restrictKernel,
                cl::NullRange,
                cl::NDRange(newSize.x(), newSize.y(), newSize.z()),
                cl::NullRange
        );
        queue.enqueueCopyBufferToImage(
                v_2_buffer,
                v_2,
                0,
                createOrigoRegion(),
                createRegion(newSize.x(), newSize.y(), newSize.z())
        );
    } else {
        restrictKernel.setArg(0, v);
        restrictKernel.setArg(1, v_2);
        queue.enqueueNDRangeKernel(
                restrictKernel,
                cl::NullRange,
                cl::NDRange(newSize.x(),newSize.y(),newSize.z()),
                cl::NullRange
        );
    }

    return v_2;
}

cl::Image3D MultigridGradientVectorFlow::prolongateVolume(
        cl::Image3D &v_l,
        cl::Image3D &v_l_p1,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D v_2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel prolongateKernel(mProgram, "prolongate");
    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Buffer v_2_buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z()
        );
        prolongateKernel.setArg(0, v_l);
        prolongateKernel.setArg(1, v_l_p1);
        prolongateKernel.setArg(2, v_2_buffer);
        queue.enqueueNDRangeKernel(
                prolongateKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

        queue.enqueueCopyBufferToImage(
                v_2_buffer,
                v_2,
                0,
                createOrigoRegion(),
                createRegion(size.x(), size.y(), size.z())
        );
    } else {
        prolongateKernel.setArg(0, v_l);
        prolongateKernel.setArg(1, v_l_p1);
        prolongateKernel.setArg(2, v_2);
        queue.enqueueNDRangeKernel(
                prolongateKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
    }

    return v_2;
}

cl::Image3D MultigridGradientVectorFlow::prolongateVolume2(
        cl::Image3D &v_l_p1,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D v_2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel prolongateKernel(mProgram, "prolongate2");
    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Buffer v_2_buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z()
        );
        prolongateKernel.setArg(0, v_l_p1);
        prolongateKernel.setArg(1, v_2_buffer);
        queue.enqueueNDRangeKernel(
                prolongateKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

        queue.enqueueCopyBufferToImage(
                v_2_buffer,
                v_2,
                0,
                createOrigoRegion(),
                createRegion(size.x(), size.y(), size.z())
        );
    } else {
        prolongateKernel.setArg(0, v_l_p1);
        prolongateKernel.setArg(1, v_2);
        queue.enqueueNDRangeKernel(
                prolongateKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
    }

    return v_2;
}


cl::Image3D MultigridGradientVectorFlow::residual(
        cl::Image3D &r,
        cl::Image3D &v,
        cl::Image3D &sqrMag,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D newResidual(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel residualKernel(mProgram, "residual");
    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Buffer newResidualBuffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z()
        );
        residualKernel.setArg(0, r);
        residualKernel.setArg(1, v);
        residualKernel.setArg(2, sqrMag);
        residualKernel.setArg(3, mu);
        residualKernel.setArg(4, spacing);
        residualKernel.setArg(5, newResidualBuffer);
        queue.enqueueNDRangeKernel(
                residualKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

        queue.enqueueCopyBufferToImage(
                newResidualBuffer,
                newResidual,
                0,
                createOrigoRegion(),
                createRegion(size.x(), size.y(), size.z())
        );
    } else {
        residualKernel.setArg(0, r);
        residualKernel.setArg(1, v);
        residualKernel.setArg(2, sqrMag);
        residualKernel.setArg(3, mu);
        residualKernel.setArg(4, spacing);
        residualKernel.setArg(5, newResidual);
        queue.enqueueNDRangeKernel(
                residualKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
    }
    return newResidual;
}


inline Vector3ui calculateNewSize(Vector3ui size) {
    bool sizeIsOkay = false;
    if(size.x() == size.y() && size.x() == size.z()) {
        float p = (float)log((float)size.x()) / log(2.0f);
        if(floor(p) == p)
            sizeIsOkay = true;
    }
    int newSize;
    if(!sizeIsOkay) {
        int maxSize = std::max(size.x(), std::max(size.y(), size.z()));
        int i = 1;
        while(true) {
            if(pow(2.0f, (float)i) >= maxSize) {
                newSize = (int)pow(2.0f, (float)(i-1));
                break;
            }
            i++;
        }
    } else {
        newSize = size.x() / 2;
    }

    return Vector3ui(newSize,newSize,newSize);

}

void MultigridGradientVectorFlow::multigridVcycle(
        cl::Image3D &r_l,
        cl::Image3D &v_l,
        cl::Image3D &sqrMag,
        int l,
        int v1,
        int v2,
        int l_max,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {

    // Pre-smoothing
    gaussSeidelSmoothing(v_l,r_l,sqrMag,v1,size,mu,spacing,imageType,bufferSize);

    if(l < l_max) {
        Vector3ui newSize = calculateNewSize(size);

        // Compute new residual
        cl::Image3D p_l = residual(r_l, v_l, sqrMag, mu, spacing, size,imageType,bufferSize);

        // Restrict residual
        cl::Image3D r_l_p1 = restrictVolume(p_l, newSize,imageType,bufferSize);

        // Restrict sqrMag
        cl::Image3D sqrMag_l_p1 = restrictVolume(sqrMag, newSize,imageType,bufferSize);

        // Initialize v_l_p1
        cl::Image3D v_l_p1 = initSolutionToZero(newSize,imageType,bufferSize);

        // Solve recursively
        multigridVcycle(r_l_p1, v_l_p1, sqrMag_l_p1, l+1,v1,v2,l_max,mu,spacing*2,newSize,imageType,bufferSize);

        // Prolongate
        v_l = prolongateVolume(v_l, v_l_p1, size,imageType,bufferSize);
    }

    // Post-smoothing
    gaussSeidelSmoothing(v_l,r_l,sqrMag,v2,size,mu,spacing,imageType,bufferSize);
}

cl::Image3D MultigridGradientVectorFlow::computeNewResidual(
        cl::Image3D &f,
        cl::Image3D &vectorField,
        float mu,
        float spacing,
        int component,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D newResidual(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel residualKernel(mProgram, "fmgResidual");
    if(!device->isWritingTo3DTexturesSupported()) {
        cl::Buffer newResidualBuffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                bufferSize*size.x()*size.y()*size.z()
        );
        residualKernel.setArg(0,vectorField);
        residualKernel.setArg(1, f);
        residualKernel.setArg(2, mu);
        residualKernel.setArg(3, spacing);
        residualKernel.setArg(4, component);
        residualKernel.setArg(5, newResidualBuffer);
        queue.enqueueNDRangeKernel(
                residualKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

        queue.enqueueCopyBufferToImage(
                newResidualBuffer,
                newResidual,
                0,
                createOrigoRegion(),
                createRegion(size.x(), size.y(), size.z())
        );
    } else {
        residualKernel.setArg(0,vectorField);
        residualKernel.setArg(1, f);
        residualKernel.setArg(2, mu);
        residualKernel.setArg(3, spacing);
        residualKernel.setArg(4, component);
        residualKernel.setArg(5, newResidual);
        queue.enqueueNDRangeKernel(
                residualKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
    }

    return newResidual;
}

cl::Image3D MultigridGradientVectorFlow::fullMultigrid(
        cl::Image3D &r_l,
        cl::Image3D &sqrMag,
        int l,
        int v0,
        int v1,
        int v2,
        int l_max,
        float mu,
        float spacing,
        Vector3ui size,
        int imageType,
        int bufferSize
        ) {
    cl::Image3D v_l;
    if(l < l_max) {
        Vector3ui newSize = calculateNewSize(size);
        cl::Image3D r_l_p1 = restrictVolume(r_l, newSize, imageType,bufferSize);
        cl::Image3D sqrMag_l = restrictVolume(sqrMag,newSize,imageType,bufferSize);
        cl::Image3D v_l_p1 = fullMultigrid(r_l_p1,sqrMag_l,l+1,v0,v1,v2,l_max,mu,spacing*2,newSize, imageType,bufferSize);
        v_l = prolongateVolume2(v_l_p1, size,imageType,bufferSize);
    } else {
        v_l = initSolutionToZero(size,imageType,bufferSize);
    }

    for(int i = 0; i < v0; i++) {
        multigridVcycle(r_l,v_l,sqrMag,l,v1,v2,l_max,mu,spacing,size,imageType,bufferSize);
    }

    return v_l;

}

void MultigridGradientVectorFlow::setIterations(uint iterations) {
    if(iterations == 0)
        throw Exception("Number of iterations can't be zero in MultigridGradientVectorFlow.");
    mIterations = iterations;
}

void MultigridGradientVectorFlow::setMuConstant(float mu) {
    if(mu > 0.2 || mu < 0)
        throw Exception("The constant mu must be larger than 0 and smaller than 0.2 in MultigridGradientVectorFlow.");
    mMu = mu;
}

float MultigridGradientVectorFlow::getMuConstant() const {
    return mMu;
}

void MultigridGradientVectorFlow::set16bitStorageFormat() {
    mUse16bitFormat = true;
}

void MultigridGradientVectorFlow::set32bitStorageFormat() {
    mUse16bitFormat = false;
}

MultigridGradientVectorFlow::MultigridGradientVectorFlow() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/GradientVectorFlow/MultigridGradientVectorFlow.cl");
    mIterations = 10;
    mMu = 0.1f;
    mUse16bitFormat = true;
}

void MultigridGradientVectorFlow::execute() {
    Image::pointer input = getInputData<Image>();
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    if((input->getDimensions() == 2 && input->getNrOfChannels() != 2) ||
            (input->getDimensions() == 3 && input->getNrOfChannels() != 3)) {
        throw Exception("Input to EulerGradientVectorFlow must be a vector field. Nr of components > 1.");
    }

    // Create output, currently only type float is output, not normalized 16 bit
    Image::pointer output = getOutputData<Image>();
    output->create(input->getSize(), TYPE_FLOAT, input->getNrOfChannels());
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);


    if(input->getDimensions() == 2) {
        throw Exception("The multigrid GVF only supports 3D");
    } else {
        if(mUse16bitFormat) {
            mProgram = getOpenCLProgram(device, "", "-DVECTORS_16BIT");
        } else {
            mProgram = getOpenCLProgram(device);
        }
        execute3DGVF(input, output, mIterations);
    }
}

void MultigridGradientVectorFlow::execute3DGVF(SharedPointer<Image> input,
        SharedPointer<Image> output, uint iterations) {
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    const Vector3f inputSpacing = input->getSpacing();
    cl::CommandQueue queue = device->getCommandQueue();
    Vector3ui size = input->getSize();
    const bool no3Dwrite = !device->isWritingTo3DTexturesSupported();
    const int totalSize = size.x()*size.y()*size.z();
    int imageType, bufferTypeSize;
    if(mUse16bitFormat) {
        imageType = CL_SNORM_INT16;
        bufferTypeSize = sizeof(short);
    } else {
        imageType = CL_FLOAT;
        bufferTypeSize = sizeof(float);
    }

    cl::Kernel initKernel(mProgram, "MGGVFInit");

    int v0 = 1;
    int v1 = 2;
    int v2 = 2;
    int l_max = log(size.maxCoeff())/log(2) - 2; // log - 1 gives error on 32 bit. Why??

    // create sqrMag
    cl::Kernel createSqrMagKernel(mProgram, "createSqrMag");
    cl::Image3D sqrMag(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );
    cl::size_t<3> offset = createOrigoRegion();
    cl::size_t<3> region = createRegion(size.x(), size.y(), size.z());

    if(no3Dwrite) {
        cl::Buffer sqrMagBuffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                totalSize*bufferTypeSize
        );
        createSqrMagKernel.setArg(0, *inputAccess->get3DImage());
        createSqrMagKernel.setArg(1, sqrMagBuffer);
        queue.enqueueNDRangeKernel(
                createSqrMagKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
        queue.enqueueCopyBufferToImage(sqrMagBuffer,sqrMag,0,offset,region);
    } else {
        createSqrMagKernel.setArg(0, *inputAccess->get3DImage());
        createSqrMagKernel.setArg(1, sqrMag);
        queue.enqueueNDRangeKernel(
                createSqrMagKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
    }
    std::cout << "sqrMag created" << std::endl;

    cl::Kernel addKernel(mProgram, "addTwoImages");
    float spacing = inputSpacing.x();
    cl::Image3D fx = initSolutionToZero(size,imageType,bufferTypeSize);

    // X component
    for(int i = 0; i < iterations; i++) {
        cl::Image3D rx = computeNewResidual(fx,*inputAccess->get3DImage(),mMu,spacing,1,size,imageType,bufferTypeSize);
        cl::Image3D fx2 = fullMultigrid(rx,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize);
        queue.finish();
        if(no3Dwrite) {
            cl::Buffer fx3(
                    device->getContext(),
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fx);
            addKernel.setArg(1,fx2);
            addKernel.setArg(2,fx3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.enqueueCopyBufferToImage(fx3,fx,0,offset,region);
            queue.finish();
        } else {
            cl::Image3D fx3(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, imageType),
                size.x(),
                size.y(),
                size.z()
            );

            addKernel.setArg(0,fx);
            addKernel.setArg(1,fx2);
            addKernel.setArg(2,fx3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.finish();

            fx = fx3;
        }

    }
    std::cout << "fx finished" << std::endl;
    spacing = inputSpacing.y();

    // create fy and ry
    // Y component
    cl::Image3D fy = initSolutionToZero(size,imageType,bufferTypeSize);
    for(int i = 0; i < iterations; i++) {
        cl::Image3D ry = computeNewResidual(fy,*inputAccess->get3DImage(),mMu,spacing,2,size,imageType,bufferTypeSize);
        cl::Image3D fy2 = fullMultigrid(ry,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize);
        queue.finish();
        if(no3Dwrite) {
            cl::Buffer fy3(
                    device->getContext(),
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fy);
            addKernel.setArg(1,fy2);
            addKernel.setArg(2,fy3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.enqueueCopyBufferToImage(fy3,fy,0,offset,region);
            queue.finish();
        } else {
            cl::Image3D fy3(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, imageType),
                size.x(),
                size.y(),
                size.z()
            );

            addKernel.setArg(0,fy);
            addKernel.setArg(1,fy2);
            addKernel.setArg(2,fy3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.finish();

            fy = fy3;
        }

    }

    std::cout << "fy finished" << std::endl;
    spacing = inputSpacing.z();

    // create fz and rz
    // Z component
    cl::Image3D fz = initSolutionToZero(size,imageType,bufferTypeSize);
    for(int i = 0; i < iterations; i++) {
        cl::Image3D rz = computeNewResidual(fz,*inputAccess->get3DImage(),mMu,spacing,3,size,imageType,bufferTypeSize);
        cl::Image3D fz2 = fullMultigrid(rz,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize);
        queue.finish();
        if(no3Dwrite) {
            cl::Buffer fz3(
                    device->getContext(),
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fz);
            addKernel.setArg(1,fz2);
            addKernel.setArg(2,fz3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.enqueueCopyBufferToImage(fz3,fz,0,offset,region);
            queue.finish();
        } else {
            cl::Image3D fz3(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, imageType),
                size.x(),
                size.y(),
                size.z()
            );

            addKernel.setArg(0,fz);
            addKernel.setArg(1,fz2);
            addKernel.setArg(2,fz3);
            queue.enqueueNDRangeKernel(
                    addKernel,
                    cl::NullRange,
                    cl::NDRange(size.x(),size.y(),size.z()),
                    cl::NullRange
            );
            queue.finish();

            fz = fz3;
        }

    }

    std::cout << "fz finished" << std::endl;


    cl::Kernel finalizeKernel(mProgram, "MGGVFFinish");
    if(no3Dwrite) {
        OpenCLImageAccess::pointer access = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Buffer finalVectorFieldBuffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                //4*totalSize*bufferTypeSize
                4 * totalSize*sizeof(float)
        );

        finalizeKernel.setArg(0, fx);
        finalizeKernel.setArg(1, fy);
        finalizeKernel.setArg(2, fz);
        finalizeKernel.setArg(3, finalVectorFieldBuffer);
        queue.enqueueNDRangeKernel(
                finalizeKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );
        queue.enqueueCopyBufferToImage(finalVectorFieldBuffer,*access->get3DImage(),0,offset,region);
    } else {
        OpenCLImageAccess::pointer access = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        finalizeKernel.setArg(0, fx);
        finalizeKernel.setArg(1, fy);
        finalizeKernel.setArg(2, fz);
        finalizeKernel.setArg(3, *access->get3DImage());
        queue.enqueueNDRangeKernel(
                finalizeKernel,
                cl::NullRange,
                cl::NDRange(size.x(),size.y(),size.z()),
                cl::NullRange
        );

    }
    std::cout << "MG GVF finished" << std::endl;
}

} // end namespace fast
