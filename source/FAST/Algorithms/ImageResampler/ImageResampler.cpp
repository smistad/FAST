#include "ImageResampler.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"

namespace fast {

ImageResampler::ImageResampler() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler3D.cl", "3D");

    mSpacing = Vector3f(-1, -1, -1);
    mInterpolationSet = false;
    mInterpolation = true;
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
    mSpacing.z() = 1.0f;
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY, float spacingZ) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
    mSpacing.z() = spacingZ;
}

void ImageResampler::execute() {
    if(mSpacing.x() < 0)
        throw Exception("You must set output spacing with setOutputSpacing before executing the ImageResampler");

    Image::pointer input = getInputData<Image>();
    Image::pointer output = getOutputData<Image>();

    Vector3f inputSpacing = input->getSpacing();

    Vector3f scale = inputSpacing.cwiseQuotient(mSpacing);

    Vector3f newSize = scale.cwiseProduct(input->getSize().cast<float>());
    int width = round(newSize.x());
    int height = round(newSize.y());
    int depth = round(newSize.z());

    if(input->getDimensions() == 2) {
        reportInfo() << "New size for volume in image resampler: " << width << " " << height << reportEnd();
        output->create(width, height, input->getDataType(), 1);
    } else {
        reportInfo() << "New size for volume in image resampler: " << width << " " << height << " " << depth << reportEnd();
        output->create(width, height, depth, input->getDataType(), 1);
    }
    output->setSpacing(mSpacing);

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    uchar useInterpolation = 1;
    if(mInterpolationSet) {
        useInterpolation = mInterpolation ? 1 : 0;
    }

    if(input->getDimensions() == 2) {
        cl::Program program = getOpenCLProgram(device, "2D");
        cl::Kernel kernel(program, "resample2D");

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer access2 = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        kernel.setArg(0, *access->get2DImage());
        kernel.setArg(1, *access2->get2DImage());
        kernel.setArg(2, scale.x());
        kernel.setArg(3, scale.y());
        kernel.setArg(4, useInterpolation);

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height),
                cl::NullRange
        );
    } else {
        std::string buildOptions = std::string("-DOUTPUT_TYPE=") + getCTypeAsString(output->getDataType());
        cl::Program program = getOpenCLProgram(device, "3D", buildOptions);
        cl::Kernel kernel(program, "resample3D");

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

        kernel.setArg(0, *access->get3DImage());
        kernel.setArg(1, *access2->get());
        kernel.setArg(2, scale.x());
        kernel.setArg(3, scale.y());
        kernel.setArg(4, scale.z());
        kernel.setArg(5, useInterpolation);

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height, depth),
                cl::NullRange
        );
    }
}

void ImageResampler::setInterpolation(bool useInterpolation) {
    mInterpolationSet = true;
    mInterpolation = useInterpolation;
}

}