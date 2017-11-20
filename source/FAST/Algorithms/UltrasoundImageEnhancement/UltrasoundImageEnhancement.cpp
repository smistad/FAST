#include <FAST/Data/Image.hpp>
#include "UltrasoundImageEnhancement.hpp"

namespace fast {


UltrasoundImageEnhancement::UltrasoundImageEnhancement() {
    createInputPort<Image>(0);

    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.cl");

    int reject = 40;
    float range = (255.0f-40.0f);
    for(int x = 0; x < 256; ++x) {
        int red, green, blue;
        if(x < reject) {
            red = 0;
            green = 0;
            blue = 0;
        } else {
            red = (int)round(((float) (x - reject) / range)*255);
            green = (int)round(((float) (x - reject) / range)*255) - 1;
            blue = (int)round(((float) (x - reject) / range)*255) + 4;
        }

        red = min(max(red, 0), 255);
        green = min(max(green, 0), 255);
        blue = min(max(blue, 0), 255);
        std::cout << red << " " << green << " " << blue << std::endl;
        mColormap.push_back((uchar)red);
        mColormap.push_back((uchar)green);
        mColormap.push_back((uchar)blue);
    }
}

void UltrasoundImageEnhancement::execute() {
    Image::pointer input = getInputData<Image>(0);
    if(input->getNrOfComponents() != 1 || input->getDataType() != TYPE_UINT8) {
        throw Exception("UltrasoundImageEnhancement expects input to be of type UINT8 and have 1 channel");
    }

    Image::pointer output = getOutputData<Image>(0);
    output->create(input->getSize(), TYPE_UINT8, 3); // Make color image
    output->setSpacing(input->getSpacing());

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Kernel kernel(getOpenCLProgram(device), "enhance");

    // Create buffer for colormap
    if(mColormap.size() != 256*3)
        throw Exception("Colormap given to UltrasoundImageEnhancement has the wrong size. It should be 256*3");
    cl::Buffer colormapBuffer = cl::Buffer(device->getContext(), CL_MEM_COPY_HOST_PTR, 256*3*sizeof(uchar), mColormap.data());

    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, colormapBuffer);

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );
}

}