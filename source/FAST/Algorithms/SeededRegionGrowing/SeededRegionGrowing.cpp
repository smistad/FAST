#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/SceneGraph.hpp"
#include <stack>
#include "FAST/Data/Image.hpp"

namespace fast {

void SeededRegionGrowing::setIntensityRange(float min, float max) {
    if(min >= max)
        throw Exception("Min must be smaller than max intensity range in SeededRegionGrowing");

    mMinimumIntensity = min;
    mMaximumIntensity = max;
    mIsModified = true;
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y) {
    Vector3i pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = 0;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y, uint z) {
    Vector3i pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(Vector3i position) {
    mSeedPoints.push_back(position);
}

SeededRegionGrowing::SeededRegionGrowing() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    mDimensionCLCodeCompiledFor = 0;
}

SeededRegionGrowing::SeededRegionGrowing(float intensityMinimum, float intensityMaximum,
                                         std::vector<Vector3i> seedPoints) : SeededRegionGrowing() {
    setIntensityRange(intensityMinimum, intensityMaximum);
    mSeedPoints = seedPoints;
}

void SeededRegionGrowing::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if(input->getDimensions() == mDimensionCLCodeCompiledFor &&
            input->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    std::string buildOptions = "";
    if(input->getDataType() == TYPE_FLOAT) {
        buildOptions = "-DTYPE_FLOAT";
    } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
        buildOptions = "-DTYPE_INT";
    } else {
        buildOptions = "-DTYPE_UINT";
    }
    std::string filename;
    if(input->getDimensions() == 2) {
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing2D.cl";
    } else {
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing3D.cl";
    }
    int programNr = device->createProgramFromSource(Config::getKernelSourcePath() + filename, buildOptions);
    mKernel = cl::Kernel(device->getProgram(programNr), "seededRegionGrowing");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}

template <class T>
void SeededRegionGrowing::executeOnHost(T* input, Image::pointer output) {
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ_WRITE);
    uchar* outputData = (uchar*)outputAccess->get();
    // initialize output to all zero
    memset(outputData, 0, output->getWidth()*output->getHeight()*output->getDepth());
    std::stack<Vector3i> queue;

    // Add seeds to queue
    for(int i = 0; i < mSeedPoints.size(); i++) {
        Vector3i pos = mSeedPoints[i];

        // Check if seed point is in bounds
        if(pos.x() < 0 || pos.y() < 0 || pos.z() < 0 ||
            pos.x() >= output->getWidth() || pos.y() >= output->getHeight() || pos.z() >= output->getDepth())
            throw Exception("One of the seed points given to SeededRegionGrowing was out of bounds.");

        queue.push(pos.cast<int>());
    }

    std::vector<Vector3i> neighborhood;
    if(output->getDimensions() == 3) {
        for(int a = -1; a < 2; a++) {
            for(int b = -1; b < 2; b++) {
                for(int c = -1; c < 2; c++) {
                    if(abs(a) + abs(b) + abs(c) != 1) // connectivity
                        continue;
                    neighborhood.push_back(Vector3i(a, b, c));
                }
            }
        }
    } else {
        for(int a = -1; a < 2; a++) {
            for(int b = -1; b < 2; b++) {
                if(abs(a) + abs(b) != 1) // connectivity
                    continue;
                neighborhood.push_back(Vector3i(a, b, 0));
            }
        }
    }

    // Process queue
    while(!queue.empty()) {
        Vector3i pos = queue.top();
        queue.pop();

        // Add neighbors to queue
        for(auto offset : neighborhood) {
            Vector3i neighbor = pos.cast<int>() + offset;
            // Check for out of bounds
            if(neighbor.x() < 0 || neighbor.y() < 0 || neighbor.z() < 0 ||
                neighbor.x() >= output->getWidth() || neighbor.y() >= output->getHeight() || neighbor.z() >= output->getDepth())
                continue;

            // Check that voxel is not already segmented
            if(outputData[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()] == 1)
                continue;

            // Check condition
            T value = input[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()];
            if(value >= mMinimumIntensity && value <= mMaximumIntensity) {
                // add it to segmentation
                outputData[neighbor.x()+neighbor.y()*output->getWidth()+neighbor.z()*output->getWidth()*output->getHeight()] = 1;

                // Add to queue
                queue.push(neighbor);
            }
        }
    }
}

void SeededRegionGrowing::execute() {
    if(mSeedPoints.size() == 0)
        throw Exception("No seed points supplied to SeededRegionGrowing");

    Image::pointer input = getInputData<Image>();
    if(input->getNrOfChannels() != 1)
        throw Exception("Seeded region growing currently doesn't support images with several components.");

    auto output = Image::createSegmentationFromImage(input);

    if(getMainDevice()->isHost()) {
        ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
        void* inputData = inputAccess->get();
        switch(input->getDataType()) {
            fastSwitchTypeMacro(executeOnHost<FAST_TYPE>((FAST_TYPE*)inputData, output));
        }
    } else {
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

        recompileOpenCLCode(input);

        ImageAccess::pointer access = output->getImageAccess(ACCESS_READ_WRITE);
        uchar* outputData = (uchar*)access->get();
        // Initialize to all 0s
        memset(outputData,0,sizeof(uchar)*output->getWidth()*output->getHeight()*output->getDepth());

        // Add sedd points
        for(int i = 0; i < mSeedPoints.size(); i++) {
            Vector3i pos = mSeedPoints[i];

            // Check if seed point is in bounds
            if(pos.x() < 0 || pos.y() < 0 || pos.z() < 0 ||
                pos.x() >= output->getWidth() || pos.y() >= output->getHeight() || pos.z() >= output->getDepth())
                throw Exception("One of the seed points given to SeededRegionGrowing was out of bounds.");

            outputData[pos.x() + pos.y()*output->getWidth() + pos.z()*output->getWidth()*output->getHeight()] = 2;
        }
        access->release();

        cl::NDRange globalSize;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        if(output->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());
            mKernel.setArg(0, *inputAccess->get2DImage());
        } else {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(), input->getDepth());
            mKernel.setArg(0, *inputAccess->get3DImage());
        }

        OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        cl::Buffer stopGrowingBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_WRITE,
                sizeof(char));
        cl::CommandQueue queue = device->getCommandQueue();
        mKernel.setArg(1, *outputAccess->get());
        mKernel.setArg(2, stopGrowingBuffer);
        mKernel.setArg(3, mMinimumIntensity);
        mKernel.setArg(4, mMaximumIntensity);

        bool stopGrowing = false;
        char stopGrowingInit = 1;
        char stopGrowingResult;
        int iterations = 0;
        do {
            iterations++;
            queue.enqueueWriteBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), &stopGrowingInit);

            queue.enqueueNDRangeKernel(
                    mKernel,
                    cl::NullRange,
                    globalSize,
                    cl::NullRange
            );

            queue.enqueueReadBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), &stopGrowingResult);
            if(stopGrowingResult == 1)
                stopGrowing = true;
        } while(!stopGrowing);
    }

    addOutputData(0, output);
}

void SeededRegionGrowing::waitToFinish() {
    if(!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        device->getCommandQueue().finish();
    }
}

std::vector<Vector3i> SeededRegionGrowing::getSeedPoints() const {
    return mSeedPoints;
}

} // end namespace fast
