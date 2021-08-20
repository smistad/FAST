#include "DoubleFilter.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

DoubleFilter::DoubleFilter() {
    // This creates an input port of type Image.
    createInputPort<Image>(0);

    // This creates an output port of type Image.
    // The OUTPUT_DEPENDS_ON_INPUT flag means that if the input
    // is a static image, the output will also be a static image,
    // and if the input is a dynamic image, the output will also
    // be a dynamic image.
    createOutputPort<Image>(0);

    // This creates an OpenCL program from a source file on disk
    createOpenCLProgram(Config::getKernelSourcePath() + "Tests/Algorithms/DoubleFilter.cl");
}

/*
 * This function performs the filter serially on the Host using plain old C++.
 * It is templated so that it can support images of different data types.
 */
template<class T>
inline void executeAlgorithmOnHost(Image::pointer input, Image::pointer output) {
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess->get();
    T * outputData = (T*)outputAccess->get();

    unsigned int nrOfElements = input->getWidth()*input->getHeight()*input->getDepth()*input->getNrOfChannels();
    for(unsigned int i = 0; i < nrOfElements; i++) {
        outputData[i] = 2.0*inputData[i];
    }
}

void DoubleFilter::execute() {
    // Get input and output data
    auto input = getInputData<Image>();

    // Initialize output image
    auto output = Image::createFromImage(input);

    if(getMainDevice()->isHost()) {
        // Execution device is Host, use the executeAlgorithmOnHost function with the given data type
        switch(input->getDataType()) {
            // This macro creates a case statement for each data type and sets FAST_TYPE to the correct C++ data type
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output));
        }
    } else {
        // Execution device is an OpenCL device
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

        // Set build options based on the data type of the data
        std::string buildOptions = "-DTYPE=" + getCTypeAsString(input->getDataType());

        // Compile the code
        cl::Kernel kernel = cl::Kernel(getOpenCLProgram(device, "", buildOptions), "doubleFilter");

        // Get global size for the kernel
        cl::NDRange globalSize(input->getWidth()*input->getHeight()*input->getDepth()*input->getNrOfChannels());

        // Set the arguments for the kernel
        OpenCLBufferAccess::pointer inputAccess = input->getOpenCLBufferAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess->get());
        kernel.setArg(1, *outputAccess->get());

        // Execute the kernel
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );
    }
    addOutputData(0, output);
}

} // end namespace fast
