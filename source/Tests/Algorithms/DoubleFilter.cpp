#include "DoubleFilter.hpp"

namespace fast {

void DoubleFilter::setInput(ImageData::pointer image) {
    setInputData(0, image);
}

ImageData::pointer DoubleFilter::getOutput() {
    return getOutputData<Image, DynamicImage>(0);
}

DoubleFilter::DoubleFilter() {
    setOutputDataDynamicDependsOnInputData(0, 0);
}

/*
 * This function performs the filter serially on the Host using plain old C++.
 * It is templated so that it can support images of different data types.
 */
template<class T>
inline void executeAlgorithmOnHost(Image::pointer input, Image::pointer output) {
    ImageAccess inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess.get();
    T * outputData = (T*)outputAccess.get();

    unsigned int nrOfElements = input->getWidth()*input->getHeight()*input->getDepth()*input->getNrOfComponents();
    for(unsigned int i = 0; i < nrOfElements; i++) {
        outputData[i] = 2.0*inputData[i];
    }
}

void DoubleFilter::execute() {
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);

    // Initialize output image
    output->createFromImage(input, getMainDevice());

    if(getMainDevice()->isHost()) {
        // Execution device is Host, use the executeAlgorithmOnHost function with the given data type
        switch(input->getDataType()) {
            // This macro creates a case statement for each data type and sets FAST_TYPE to the correct C++ data type
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output));
        }
    } else {
        // Execution device is an OpenCL device
        OpenCLDevice::pointer device = getMainDevice();

        // Set build options based on the data type of the data
        std::string buildOptions = "";
        switch(input->getDataType()) {
        case TYPE_FLOAT:
            buildOptions = "-DTYPE=float";
            break;
        case TYPE_INT8:
            buildOptions = "-DTYPE=char";
            break;
        case TYPE_UINT8:
            buildOptions = "-DTYPE=uchar";
            break;
        case TYPE_INT16:
            buildOptions = "-DTYPE=short";
            break;
        case TYPE_UINT16:
            buildOptions = "-DTYPE=ushort";
            break;
        }

        // Compile the code
        int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Tests/Algorithms/DoubleFilter.cl", buildOptions);
        cl::Kernel kernel = cl::Kernel(device->getProgram(programNr), "doubleFilter");

        // Get global size for the kernel
        cl::NDRange globalSize(input->getWidth()*input->getHeight()*input->getDepth()*input->getNrOfComponents());

        // Set the arguments for the kernel
        OpenCLBufferAccess inputAccess = input->getOpenCLBufferAccess(ACCESS_READ, device);
        OpenCLBufferAccess outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess.get());
        kernel.setArg(1, *outputAccess.get());

        // Execute the kernel
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );
    }
}

}
