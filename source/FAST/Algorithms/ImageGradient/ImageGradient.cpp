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
        output->create2DImage(
                input->getWidth(),
                input->getHeight(),
                TYPE_FLOAT,
                2,
                getMainDevice()
                );
    } else {
         output->create3DImage(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_FLOAT,
                3,
                getMainDevice()
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
        if(input->getDimensions() == 2) {
            kernel = cl::Kernel(program, "gradient2D");
            OpenCLImageAccess2D::pointer inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D::pointer outputAccess = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess->get());
            kernel.setArg(1, *outputAccess->get());
        } else {
            kernel = cl::Kernel(program, "gradient3D");
            OpenCLImageAccess3D::pointer inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            kernel.setArg(0, *inputAccess->get());

            const bool writingTo3DTextures = device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
            if(writingTo3DTextures) {
                OpenCLImageAccess3D::pointer outputAccess = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
                kernel.setArg(1, *outputAccess->get());
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

