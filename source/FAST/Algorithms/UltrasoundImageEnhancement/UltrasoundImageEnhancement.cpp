#include <FAST/Data/Image.hpp>
#include "UltrasoundImageEnhancement.hpp"

namespace fast {


UltrasoundImageEnhancement::UltrasoundImageEnhancement() {
    createInputPort<Image>(0);

    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.cl");

    createIntegerAttribute("reject", "Reject", "How many intensity values at bottom to reject.", 40);

    mColormapUploaded = false;

    setReject(40);
}

void UltrasoundImageEnhancement::execute() {
    Image::pointer input = getInputData<Image>(0);
    if(input->getDataType() != TYPE_UINT8) {
        throw Exception("UltrasoundImageEnhancement expects input to be of type UINT8");
    }

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    if(!mColormapUploaded) {
        mColormapBuffer = cl::Buffer(device->getContext(), CL_MEM_COPY_HOST_PTR, 256*3*sizeof(uchar), mColormap.data());
        mColormapUploaded = true;
    }

    Image::pointer output = getOutputData<Image>(0);
    output->create(input->getSize(), TYPE_UINT8, 3); // Make color image
    output->setSpacing(input->getSpacing());

    cl::CommandQueue queue = device->getCommandQueue();
    cl::Kernel kernel(getOpenCLProgram(device), "enhance");


    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, mColormapBuffer);

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );
}

void UltrasoundImageEnhancement::loadAttributes() {
    setReject(getIntegerAttribute("reject"));
}

void UltrasoundImageEnhancement::setReject(int reject) {
    // Create colormap based on reject
    float range = (255.0f - (float)reject);
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
        mColormap.push_back((uchar)red);
        mColormap.push_back((uchar)green);
        mColormap.push_back((uchar)blue);
    }

    mColormapUploaded = false;
}

}