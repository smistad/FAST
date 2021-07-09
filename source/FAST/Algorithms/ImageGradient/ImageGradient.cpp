#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

ImageGradient::ImageGradient(bool use16bit) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageGradient/ImageGradient.cl");

    mUse16bitFormat = use16bit;
}

void ImageGradient::execute() {
    auto input = getInputData<Image>(0);

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;
    if(mUse16bitFormat) {
        type = TYPE_SNORM_INT16;
        buildOptions = "-DVECTORS_16BIT";
    }

    // Initialize output image
    Image::pointer output;
    if(input->getDimensions() == 2) {
        output = Image::create(
                input->getWidth(),
                input->getHeight(),
                type,
                2
        );
    } else {
         output = Image::create(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                type,
                3
        );
    }

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        if(input->getDimensions() == 2) {
            kernel = cl::Kernel(program, "gradient2D");
            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess->get2DImage());
            kernel.setArg(1, *outputAccess->get2DImage());
        } else {
            kernel = cl::Kernel(program, "gradient3D");
            kernel.setArg(0, *inputAccess->get3DImage());

            if(device->isWritingTo3DTexturesSupported()) {
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *outputAccess->get3DImage());
            } else {
                // If device does not support writing to 3D textures, use a buffer instead
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *outputAccess->get());
            }
        }

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
    }
    addOutputData(0, output);
}

void ImageGradient::set16bitStorageFormat() {
    mUse16bitFormat = true;
}

void ImageGradient::set32bitStorageFormat() {
    mUse16bitFormat = false;
}

}


