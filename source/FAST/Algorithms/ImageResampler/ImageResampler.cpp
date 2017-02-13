#include "ImageResampler.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageResampler::ImageResampler() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler.cl");

    mSpacing = Vector3f(-1, -1, -1);
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY, float spacingZ) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
    mSpacing.z() = spacingZ;
}

void ImageResampler::execute() {
    Image::pointer input = getStaticInputData<Image>();
    Image::pointer output = getStaticOutputData<Image>();

    Vector3f inputSpacing = input->getSpacing();

    Vector3f scale = inputSpacing.cwiseQuotient(mSpacing);

    Vector3f newSize = scale.cwiseProduct(input->getSize().cast<float>());
    int width = round(newSize.x());
    int height = round(newSize.y());
    int depth = round(newSize.z());
    reportInfo() << "New size for volume in image resampler: " << width << " " << height << " " << depth << reportEnd();

    output->create(width, height, depth, input->getDataType(), 1);
    output->setSpacing(mSpacing);

    OpenCLDevice::pointer device = getMainDevice();
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "resample3D");

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLBufferAccess::pointer access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *access->get3DImage());
    kernel.setArg(1, *access2->get());
    kernel.setArg(2, scale.x());
    kernel.setArg(3, scale.y());
    kernel.setArg(4, scale.z());

    cl::CommandQueue queue = device->getCommandQueue();

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width, height, depth),
            cl::NullRange
    );
}

}