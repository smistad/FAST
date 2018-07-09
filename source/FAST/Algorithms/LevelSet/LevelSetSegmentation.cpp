#include "LevelSetSegmentation.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"

namespace fast {

LevelSetSegmentation::LevelSetSegmentation() {
    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/LevelSet/LevelSetSegmentation.cl");

    mCurvatureWeight = 0.9;
    mIntensityMeanSet = false;
    mIntensityVarianceSet = false;
    mIterations = 1000;
}

void LevelSetSegmentation::setCurvatureWeight(float weight) {
    if(weight < 0 || weight > 1)
        throw Exception("Curvature weights must be within [0, 1]");
    mCurvatureWeight = weight;
    mIsModified = true;
}

void LevelSetSegmentation::setIntensityMean(float intensity) {
    mIntensityMean = intensity;
    mIntensityMeanSet = true;
    mIsModified = true;
}

void LevelSetSegmentation::setIntensityVariance(float variance) {
    mIntensityVariance = variance;
    mIntensityVarianceSet = true;
    mIsModified = true;
}

void LevelSetSegmentation::setMaxIterations(uint iterations) {
    mIterations = iterations;
}

void LevelSetSegmentation::addSeedPoint(Vector3i position, float size) {
    mSeeds.push_back(std::make_pair(position, size));
    mIsModified = true;
}

void LevelSetSegmentation::execute() {
    if(!mIntensityMeanSet || !mIntensityVarianceSet)
        throw Exception("Intensity mean or variance not given to LevelSetSegmentation");

    Image::pointer input = getInputData<Image>();

    if(input->getDimensions() != 3)
        throw Exception("Level set segmentation only supports 3D atm");


    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Program program = getOpenCLProgram(device);

    Image::pointer phi = Image::New();
    phi->create(input->getSize(), TYPE_FLOAT, 1);

    OpenCLImageAccess::pointer phiAccess = phi->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    cl::Image3D phi_1 = *phiAccess->get3DImage();

    if(mSeeds.size() == 0)
        throw Exception("The LevelSetSegmentation algorithm must be given a seed point");

    Vector3i seedPos = mSeeds[0].first;
    reportInfo() << "Using seed: " << seedPos.transpose() << reportEnd();
    float seedRadius = mSeeds[0].second;
    Vector3ui size = input->getSize();

    // Create seed
    cl::Kernel createSeedKernel(program, "initializeLevelSetFunction");
    createSeedKernel.setArg(0, phi_1);
    createSeedKernel.setArg(1, seedPos.x());
    createSeedKernel.setArg(2, seedPos.y());
    createSeedKernel.setArg(3, seedPos.z());
    createSeedKernel.setArg(4, seedRadius);
    queue.enqueueNDRangeKernel(
            createSeedKernel,
            cl::NullRange,
            cl::NDRange(size.x(),size.y(),size.z()),
            cl::NullRange
    );

    cl::Kernel kernel(program, "updateLevelSetFunction");
    cl::size_t<3> origin = createOrigoRegion();
    cl::size_t<3> region = createRegion(size);

    Vector3f spacing = input->getSpacing();
    const float minimumSpacing = std::min(std::min(spacing.x(), spacing.y()), spacing.z());

    if(!device->isWritingTo3DTexturesSupported()) {
        // Create auxillary buffer
        cl::Buffer writeBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                sizeof(float)*size.x()*size.y()*size.z()
        );

        /*
        for(int i = 0; i < iterations; i++) {
            updateLevelSetFunction(ocl, kernel, inputData, phi_1, writeBuffer, size, threshold, epsilon, alpha);
            ocl.queue.enqueueCopyBufferToImage(
                    writeBuffer,
                    phi_1,
                    0,
                    origin,
                    region
            );
        }
         */
    } else {
        cl::Image3D phi_2 = cl::Image3D(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, CL_FLOAT),
                input->getWidth(),
                input->getHeight(),
                input->getDepth()
        );

        Image::pointer speed = Image::New();
        speed->create(input->getSize(), TYPE_FLOAT, 1);

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        kernel.setArg(0, *access->get3DImage());
        kernel.setArg(3, mIntensityMean);
        kernel.setArg(4, mIntensityVariance);
        kernel.setArg(5, mCurvatureWeight);

        float deltaT = 0.0001;
        for(int i = 0; i < mIterations; i++) {
            kernel.setArg(7, deltaT);
            reportInfo() << "Iteration: " << i << " delta t: " << deltaT << reportEnd();
            if(i % 2 == 0) {
                kernel.setArg(1, phi_1);
                kernel.setArg(2, phi_2);
            } else {
                kernel.setArg(1, phi_2);
                kernel.setArg(2, phi_1);
            }
            {
                OpenCLImageAccess::pointer speedAccess = speed->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(6, *speedAccess->get3DImage());
                queue.enqueueNDRangeKernel(
                        kernel,
                        cl::NullRange,
                        cl::NDRange(size.x(), size.y(), size.z()),
                        cl::NullRange
                );
                queue.finish();
            }

            // Calculate max speed and deltaT for next round
            deltaT = 0.5/speed->calculateMaximumIntensity();
        }
        if(mIterations % 2 != 0) {
            // Phi_2 was written to in the last iteration, copy this to the result
            queue.enqueueCopyImage(phi_2,phi_1,origin,origin,region);
        }
    }

    phiAccess->release();

    // Create segmentation from level set function
    BinaryThresholding::pointer thresholding = BinaryThresholding::New();
    thresholding->setUpperThreshold(0);
    thresholding->setInputData(phi);
    DataPort::pointer port = thresholding->getOutputPort();
    thresholding->update(0);
    Segmentation::pointer output = port->getNextFrame<Segmentation>();
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);
    addOutputData(0, output);
}

}