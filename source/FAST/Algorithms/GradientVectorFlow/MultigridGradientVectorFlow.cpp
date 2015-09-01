#include "MultigridGradientVectorFlow.hpp"
#include "FAST/Data/Image.hpp"
#include "HelperFunctions.hpp"

namespace fast {

cl::Image3D MultigridGradientVectorFlow::initSolutionToZero(Vector3ui size, int imageType, int bufferSize, bool no3Dwrite) {
    OpenCLDevice::pointer device = getMainDevice();
    cl::Image3D v(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    if(no3Dwrite) {
        /*
        Kernel initToZeroKernel(ocl.program, "initFloatBuffer");
        Buffer vBuffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);
        initToZeroKernel.setArg(0,vBuffer);
        ocl.queue.enqueueNDRangeKernel(
                initToZeroKernel,
                NullRange,
                NDRange(size.x*size.y*size.z),
                NullRange
        );
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        ocl.queue.enqueueCopyBufferToImage(vBuffer,v,0,offset,region);
        */
    } else {
        cl::Kernel initToZeroKernel(getOpenCLProgram(device), "init3DFloat");
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
        int bufferSize,
        bool no3Dwrite
        ) {

    if(iterations <= 0)
        return;
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();

    cl::Kernel gaussSeidelKernel(getOpenCLProgram(device), "GVFgaussSeidel");
    cl::Kernel gaussSeidelKernel2(getOpenCLProgram(device), "GVFgaussSeidel2");

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

    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        Buffer v_2_buffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);

        for(int i = 0; i < iterations*2; i++) {
             if(i % 2 == 0) {
                 gaussSeidelKernel.setArg(4, v);
                 gaussSeidelKernel.setArg(5, v_2_buffer);
                 ocl.queue.enqueueNDRangeKernel(
                    gaussSeidelKernel,
                    NullRange,
                    NDRange(size.x,size.y,size.z),
                    NDRange(4,4,4)
                );
                ocl.queue.enqueueCopyBufferToImage(v_2_buffer, v_2,0,offset,region);
             } else {
                 gaussSeidelKernel2.setArg(4, v_2);
                 gaussSeidelKernel2.setArg(5, v_2_buffer);
                 ocl.queue.enqueueNDRangeKernel(
                    gaussSeidelKernel2,
                    NullRange,
                    NDRange(size.x,size.y,size.z),
                    NDRange(4,4,4)
                );
                ocl.queue.enqueueCopyBufferToImage(v_2_buffer, v,0,offset,region);
             }
        }
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {

    OpenCLDevice::pointer device = getMainDevice();
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

    cl::Kernel restrictKernel(getOpenCLProgram(device), "restrictVolume");
    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = newSize.x;
        region[1] = newSize.y;
        region[2] = newSize.z;
        Buffer v_2_buffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*newSize.x*newSize.y*newSize.z);
        restrictKernel.setArg(0, v);
        restrictKernel.setArg(1, v_2_buffer);
        ocl.queue.enqueueNDRangeKernel(
                restrictKernel,
                NullRange,
                NDRange(newSize.x,newSize.y,newSize.z),
                NDRange(4,4,4)
        );
        ocl.queue.enqueueCopyBufferToImage(v_2_buffer, v_2,0,offset,region);
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D v_2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel prolongateKernel(getOpenCLProgram(device), "prolongate");
    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        Buffer v_2_buffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);
        prolongateKernel.setArg(0, v_l);
        prolongateKernel.setArg(1, v_l_p1);
        prolongateKernel.setArg(2, v_2_buffer);
        ocl.queue.enqueueNDRangeKernel(
                prolongateKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );

        ocl.queue.enqueueCopyBufferToImage(v_2_buffer, v_2,0,offset,region);
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D v_2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel prolongateKernel(getOpenCLProgram(device), "prolongate2");
    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        Buffer v_2_buffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);
        prolongateKernel.setArg(0, v_l_p1);
        prolongateKernel.setArg(1, v_2_buffer);
        ocl.queue.enqueueNDRangeKernel(
                prolongateKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );

        ocl.queue.enqueueCopyBufferToImage(v_2_buffer, v_2,0,offset,region);
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D newResidual(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel residualKernel(getOpenCLProgram(device), "residual");
    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        Buffer newResidualBuffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);
        residualKernel.setArg(0, r);
        residualKernel.setArg(1, v);
        residualKernel.setArg(2, sqrMag);
        residualKernel.setArg(3, mu);
        residualKernel.setArg(4, spacing);
        residualKernel.setArg(5, newResidualBuffer);
        ocl.queue.enqueueNDRangeKernel(
                residualKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );

        ocl.queue.enqueueCopyBufferToImage(newResidualBuffer, newResidual,0,offset,region);
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {

    // Pre-smoothing
    gaussSeidelSmoothing(v_l,r_l,sqrMag,v1,size,mu,spacing,imageType,bufferSize,no3Dwrite);

    if(l < l_max) {
        Vector3ui newSize = calculateNewSize(size);

        // Compute new residual
        cl::Image3D p_l = residual(r_l, v_l, sqrMag, mu, spacing, size,imageType,bufferSize,no3Dwrite);

        // Restrict residual
        cl::Image3D r_l_p1 = restrictVolume(p_l, newSize,imageType,bufferSize,no3Dwrite);

        // Restrict sqrMag
        cl::Image3D sqrMag_l_p1 = restrictVolume(sqrMag, newSize,imageType,bufferSize,no3Dwrite);

        // Initialize v_l_p1
        cl::Image3D v_l_p1 = initSolutionToZero(newSize,imageType,bufferSize,no3Dwrite);

        // Solve recursively
        multigridVcycle(r_l_p1, v_l_p1, sqrMag_l_p1, l+1,v1,v2,l_max,mu,spacing*2,newSize,imageType,bufferSize,no3Dwrite);

        // Prolongate
        v_l = prolongateVolume(v_l, v_l_p1, size,imageType,bufferSize,no3Dwrite);
    }

    // Post-smoothing
    gaussSeidelSmoothing(v_l,r_l,sqrMag,v2,size,mu,spacing,imageType,bufferSize,no3Dwrite);
}

cl::Image3D MultigridGradientVectorFlow::computeNewResidual(
        cl::Image3D &f,
        cl::Image3D &vectorField,
        float mu,
        float spacing,
        int component,
        Vector3ui size,
        int imageType,
        int bufferSize,
        bool no3Dwrite
        ) {
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Image3D newResidual(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );

    cl::Kernel residualKernel(getOpenCLProgram(device), "fmgResidual");
    if(no3Dwrite) {
        /*
        cl::size_t<3> offset;
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = 0;
        cl::size_t<3> region;
        region[0] = size.x;
        region[1] = size.y;
        region[2] = size.z;
        Buffer newResidualBuffer = Buffer(ocl.context, CL_MEM_WRITE_ONLY, bufferSize*size.x*size.y*size.z);
        residualKernel.setArg(0,vectorField);
        residualKernel.setArg(1, f);
        residualKernel.setArg(2, mu);
        residualKernel.setArg(3, spacing);
        residualKernel.setArg(4, component);
        residualKernel.setArg(5, newResidualBuffer);
        ocl.queue.enqueueNDRangeKernel(
                residualKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );
        ocl.queue.enqueueCopyBufferToImage(newResidualBuffer, newResidual,0,offset,region);
        */
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
        int bufferSize,
        bool no3Dwrite
        ) {
    cl::Image3D v_l;
    if(l < l_max) {
        Vector3ui newSize = calculateNewSize(size);
        cl::Image3D r_l_p1 = restrictVolume(r_l, newSize, imageType,bufferSize,no3Dwrite);
        cl::Image3D sqrMag_l = restrictVolume(sqrMag,newSize,imageType,bufferSize,no3Dwrite);
        cl::Image3D v_l_p1 = fullMultigrid(r_l_p1,sqrMag_l,l+1,v0,v1,v2,l_max,mu,spacing*2,newSize, imageType,bufferSize,no3Dwrite);
        v_l = prolongateVolume2(v_l_p1, size,imageType,bufferSize,no3Dwrite);
    } else {
        v_l = initSolutionToZero(size,imageType,bufferSize,no3Dwrite);
    }

    for(int i = 0; i < v0; i++) {
        multigridVcycle(r_l,v_l,sqrMag,l,v1,v2,l_max,mu,spacing,size,imageType,bufferSize,no3Dwrite);
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
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/GradientVectorFlow/MultigridGradientVectorFlow.cl");
    mIterations = 10;
    mMu = 0.1f;
    mUse16bitFormat = true;
}

void MultigridGradientVectorFlow::execute() {
    Image::pointer input = getStaticInputData<Image>();
    OpenCLDevice::pointer device = getMainDevice();

    if((input->getDimensions() == 2 && input->getNrOfComponents() != 2) ||
            (input->getDimensions() == 3 && input->getNrOfComponents() != 3)) {
        throw Exception("Input to EulerGradientVectorFlow must be a vector field. Nr of components > 1.");
    }

    // Create output, currently only type float is output, not normalized 16 bit
    Image::pointer output = getStaticOutputData<Image>();
    output->create(input->getSize(), TYPE_FLOAT, input->getNrOfComponents());
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);


    if(input->getDimensions() == 2) {
        throw Exception("The multigrid GVF only supports 3D");
    } else {
        if(device->isWritingTo3DTexturesSupported()) {
            execute3DGVF(input, output, mIterations);
        } else {
            execute3DGVFNo3DWrite(input, output, mIterations);
        }
    }
}

void MultigridGradientVectorFlow::execute3DGVF(SharedPointer<Image> input,
        SharedPointer<Image> output, uint iterations) {
    OpenCLDevice::pointer device = getMainDevice();
    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    cl::CommandQueue queue = device->getCommandQueue();
    Vector3ui size = input->getSize();
    const bool no3Dwrite = false;
    const int totalSize = size.x()*size.y()*size.z();
    int imageType, bufferTypeSize;
    if(mUse16bitFormat) {
        imageType = CL_SNORM_INT16;
        bufferTypeSize = sizeof(short);
    } else {
        imageType = CL_FLOAT;
        bufferTypeSize = sizeof(float);
    }

    cl::Kernel initKernel(getOpenCLProgram(device), "MGGVFInit");

    int v0 = 1;
    int v1 = 2;
    int v2 = 2;
    int l_max = 2; // TODO this should be calculated
    float spacing = 1.0f;

    // create sqrMag
    cl::Kernel createSqrMagKernel(getOpenCLProgram(device), "createSqrMag");
    cl::Image3D sqrMag(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_R, imageType),
            size.x(),
            size.y(),
            size.z()
    );
    cl::size_t<3> offset = oul::createOrigoRegion();
    cl::size_t<3> region = oul::createRegion(size.x(), size.y(), size.z());

    if(no3Dwrite) {
        /*
        Buffer sqrMagBuffer = Buffer(
                ocl.context,
                CL_MEM_WRITE_ONLY,
                totalSize*bufferTypeSize
        );
        createSqrMagKernel.setArg(0, *vectorField);
        createSqrMagKernel.setArg(1, sqrMagBuffer);
        ocl.queue.enqueueNDRangeKernel(
                createSqrMagKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );
        ocl.queue.enqueueCopyBufferToImage(sqrMagBuffer,sqrMag,0,offset,region);
        */
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

    cl::Kernel addKernel(getOpenCLProgram(device), "addTwoImages");
    cl::Image3D fx = initSolutionToZero(size,imageType,bufferTypeSize,no3Dwrite);

    // X component
    for(int i = 0; i < iterations; i++) {
        cl::Image3D rx = computeNewResidual(fx,*inputAccess->get3DImage(),mMu,spacing,1,size,imageType,bufferTypeSize,no3Dwrite);
        cl::Image3D fx2 = fullMultigrid(rx,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize,no3Dwrite);
        queue.finish();
        if(no3Dwrite) {
            /*
            Buffer fx3 = Buffer(
                    ocl.context,
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fx);
            addKernel.setArg(1,fx2);
            addKernel.setArg(2,fx3);
            ocl.queue.enqueueNDRangeKernel(
                    addKernel,
                    NullRange,
                    NDRange(size.x,size.y,size.z),
                    NDRange(4,4,4)
            );
            ocl.queue.enqueueCopyBufferToImage(fx3,fx,0,offset,region);
            ocl.queue.finish();
            */
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

    // create fy and ry
    // Y component
    cl::Image3D fy = initSolutionToZero(size,imageType,bufferTypeSize,no3Dwrite);
    for(int i = 0; i < iterations; i++) {
        cl::Image3D ry = computeNewResidual(fy,*inputAccess->get3DImage(),mMu,spacing,2,size,imageType,bufferTypeSize,no3Dwrite);
        cl::Image3D fy2 = fullMultigrid(ry,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize,no3Dwrite);
        queue.finish();
        if(no3Dwrite) {
            /*
            Buffer fy3 = Buffer(
                    ocl.context,
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fy);
            addKernel.setArg(1,fy2);
            addKernel.setArg(2,fy3);
            ocl.queue.enqueueNDRangeKernel(
                    addKernel,
                    NullRange,
                    NDRange(size.x,size.y,size.z),
                    NDRange(4,4,4)
            );
            ocl.queue.enqueueCopyBufferToImage(fy3,fy,0,offset,region);
            ocl.queue.finish();
            */
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

    // create fz and rz
    // Z component
    cl::Image3D fz = initSolutionToZero(size,imageType,bufferTypeSize,no3Dwrite);
    for(int i = 0; i < iterations; i++) {
        cl::Image3D rz = computeNewResidual(fz,*inputAccess->get3DImage(),mMu,spacing,3,size,imageType,bufferTypeSize,no3Dwrite);
        cl::Image3D fz2 = fullMultigrid(rz,sqrMag,0,v0,v1,v2,l_max,mMu,spacing,size,imageType,bufferTypeSize,no3Dwrite);
        queue.finish();
        if(no3Dwrite) {
            /*
            Buffer fz3 = Buffer(
                    ocl.context,
                    CL_MEM_WRITE_ONLY,
                    totalSize*bufferTypeSize
            );

            addKernel.setArg(0,fz);
            addKernel.setArg(1,fz2);
            addKernel.setArg(2,fz3);
            ocl.queue.enqueueNDRangeKernel(
                    addKernel,
                    NullRange,
                    NDRange(size.x,size.y,size.z),
                    NDRange(4,4,4)
            );
            ocl.queue.enqueueCopyBufferToImage(fz3,fz,0,offset,region);
            ocl.queue.finish();
            */
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


    cl::Kernel finalizeKernel(getOpenCLProgram(device), "MGGVFFinish");
    if(no3Dwrite) {
        /*
        Buffer finalVectorFieldBuffer = Buffer(
                ocl.context,
                CL_MEM_WRITE_ONLY,
                4*totalSize*bufferTypeSize
        );

        finalizeKernel.setArg(0, fx);
        finalizeKernel.setArg(1, fy);
        finalizeKernel.setArg(2, fz);
        finalizeKernel.setArg(3, finalVectorFieldBuffer);
        ocl.queue.enqueueNDRangeKernel(
                finalizeKernel,
                NullRange,
                NDRange(size.x,size.y,size.z),
                NDRange(4,4,4)
        );
        ocl.queue.enqueueCopyBufferToImage(finalVectorFieldBuffer,finalVectorField,0,offset,region);
        */
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

void MultigridGradientVectorFlow::execute3DGVFNo3DWrite(
        SharedPointer<Image> input, SharedPointer<Image> output,
        uint iterations) {
    throw Exception("Multigrid GVF not implemented for no 3d write devices yet.");
}

} // end namespace fast
