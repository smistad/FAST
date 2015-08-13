#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

ImageGradient::ImageGradient() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void ImageGradient::execute() {
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);

    // Initialize output image
    if(input->getDimensions() == 2) {
        output->create(
                input->getWidth(),
                input->getHeight(),
                TYPE_FLOAT,
                2
                );
    } else {
         output->create(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_FLOAT,
                3
                );
    }

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        std::string buildOptions = "";
        if(input->getDataType() == TYPE_FLOAT) {
            buildOptions = "-DTYPE_FLOAT";
        } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
            buildOptions = "-DTYPE_INT";
        } else {
            buildOptions = "-DTYPE_UINT";
        }
        int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/ImageGradient/ImageGradient.cl", buildOptions);
        cl::Program program = device->getProgram(programNr);
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
}


}

