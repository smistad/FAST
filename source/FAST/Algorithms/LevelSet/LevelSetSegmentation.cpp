#include "LevelSetSegmentation.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"

namespace fast {

LevelSetSegmentation::LevelSetSegmentation() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/LevelSet/LevelSetSegmentation.cl");

    mCurvatureWeight = 0.9;
    mIntensityMeanSet = false;
    mIntensityVarianceSet = false;
    mIterations = 1000;
}

LevelSetSegmentation::LevelSetSegmentation(std::vector<Vector3i> seedPoints, float seedRadius, float curvatureWeight,
                                           int iterations) : LevelSetSegmentation() {
    for(auto seed : seedPoints) {
        addSeedPoint(seed, seedRadius);
    }

    setCurvatureWeight(curvatureWeight);
    setMaxIterations(iterations);
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

    auto phi = Image::create(input->getSize(), TYPE_FLOAT, 1);


    if(mSeeds.size() == 0)
        throw Exception("The LevelSetSegmentation algorithm must be given a seed point");

    Vector3ui size = input->getSize();
    for(auto seed : mSeeds) {
        Vector3i seedPos = seed.first;
        reportInfo() << "Using seed: " << seedPos.transpose() << reportEnd();
        float seedRadius = seed.second;

        // Create seed
        cl::Kernel createSeedKernel(program, "initializeLevelSetFunction");

        if(device->isWritingTo3DTexturesSupported()) {
            auto phiAccess = phi->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            cl::Image3D phi_1 = *phiAccess->get3DImage();
            createSeedKernel.setArg(0, phi_1);
        } else {
            auto phiAccess = phi->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            createSeedKernel.setArg(0, *phiAccess->get());
        }
        createSeedKernel.setArg(1, seedPos.x());
        createSeedKernel.setArg(2, seedPos.y());
        createSeedKernel.setArg(3, seedPos.z());
        createSeedKernel.setArg(4, seedRadius);
        queue.enqueueNDRangeKernel(
                createSeedKernel,
                cl::NullRange,
                cl::NDRange(size.x(), size.y(), size.z()),
                cl::NullRange
        );
    }

    cl::Kernel kernel(program, "updateLevelSetFunction");
    cl::size_t<3> origin = createOrigoRegion();
    cl::size_t<3> region = createRegion(size);

    Vector3f spacing = input->getSpacing();
    const float minimumSpacing = std::min(std::min(spacing.x(), spacing.y()), spacing.z());

    if(!device->isWritingTo3DTexturesSupported()) {
        auto phi2 = Image::create(input->getSize(), TYPE_FLOAT, 1);
        auto speed = Image::create(input->getSize(), TYPE_FLOAT, 1);

        auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
        kernel.setArg(0, *access->get3DImage());
        kernel.setArg(3, mIntensityMean);
        kernel.setArg(4, mIntensityVariance);
        kernel.setArg(5, mCurvatureWeight);

        float deltaT = 0.0001;
        for(int i = 0; i < mIterations; i++) {
            kernel.setArg(7, deltaT);
            reportInfo() << "Iteration: " << i << " delta t: " << deltaT << reportEnd();
            if(i % 2 == 0) {
                auto access1 = phi->getOpenCLImageAccess(ACCESS_READ, device);
                auto access2 = phi2->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *access1->get3DImage());
                kernel.setArg(2, *access2->get());
            } else {
                auto access1 = phi->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                auto access2 = phi2->getOpenCLImageAccess(ACCESS_READ, device);
                kernel.setArg(1, *access2->get3DImage());
                kernel.setArg(2, *access1->get());
            }
            {
                auto speedAccess = speed->getOpenCLBufferAccess(ACCESS_READ_WRITE,device);
                kernel.setArg(6, *speedAccess->get());
                queue.enqueueNDRangeKernel(
                        kernel,
                        cl::NullRange,
                        cl::NDRange(size.x(), size.y(), size.z()),
                        cl::NullRange
                        );
                queue.finish();
            }

            // Calculate max speed and deltaT for next round
            deltaT = 0.5f/speed->calculateMaximumIntensity();
        }
        if(mIterations % 2 != 0) {
            // Phi_2 was written to in the last iteration, copy this to the result
            //auto access1 = phi->getOpenCLImageAccess(ACCESS_READ, device);
            //auto access2 = phi2->getOpenCLImageAccess(ACCESS_READ, device);
            phi = phi2->copy(device);
            //queue.enqueueCopyImage(phi_2,phi_1,origin,origin,region);
        }
    } else {
        auto phiAccess = phi->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Image3D phi_1 = *phiAccess->get3DImage();
        cl::Image3D phi_2 = cl::Image3D(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_R, CL_FLOAT),
                input->getWidth(),
                input->getHeight(),
                input->getDepth()
        );

        auto speed = Image::create(input->getSize(), TYPE_FLOAT, 1);

        auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
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

    // Create segmentation from level set function
    BinaryThresholding::pointer thresholding = BinaryThresholding::New();
    thresholding->setUpperThreshold(0);
    thresholding->setInputData(phi);
    DataChannel::pointer port = thresholding->getOutputPort();
    thresholding->update();
    auto output = port->getNextFrame<Image>();
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);
    addOutputData(0, output);
}

}