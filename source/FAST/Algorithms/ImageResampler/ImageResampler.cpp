#include "ImageResampler.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"

namespace fast {

void ImageResampler::loadAttributes() {
    setOutputSpacing(getFloatAttribute("spacing-x"), getFloatAttribute("spacing-y"), getFloatAttribute("spacing-z"));
    setInterpolation(getBooleanAttribute("interpolation"));
}

ImageResampler::ImageResampler() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler3D.cl", "3D");

    createFloatAttribute("spacing-x", "Spacing X", "Spacing X", mSpacing.x());
    createFloatAttribute("spacing-y", "Spacing Y", "Spacing Y", mSpacing.y());
    createFloatAttribute("spacing-z", "Spacing Z", "Spacing Z", mSpacing.z());
    createBooleanAttribute("interpolation", "Bilinear interpolation", "Bilinear interpolation", mInterpolation);
}

ImageResampler::ImageResampler(float spacingX, float spacingY, float spacingZ, bool useInterpolation) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageResampler/ImageResampler3D.cl", "3D");

    setOutputSpacing(spacingX, spacingY, spacingZ);
    setInterpolation(useInterpolation);
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
    mSpacing.z() = 1.0f;
    setModified(true);
}

void ImageResampler::setOutputSpacing(float spacingX, float spacingY, float spacingZ) {
    mSpacing.x() = spacingX;
    mSpacing.y() = spacingY;
    mSpacing.z() = spacingZ;
    setModified(true);
}

void ImageResampler::execute() {
    if(mSpacing.x() < 0)
        throw Exception("You must set output spacing with setOutputSpacing before executing the ImageResampler");

    auto input = getInputData<Image>();

    Vector3f inputSpacing = input->getSpacing();

    Vector3f scale = inputSpacing.cwiseQuotient(mSpacing);

    Vector3f newSize = scale.cwiseProduct(input->getSize().cast<float>());
    int width = round(newSize.x());
    int height = round(newSize.y());
    int depth = round(newSize.z());

    Image::pointer output;
    if(input->getDimensions() == 2) {
        reportInfo() << "New size for image in image resampler: " << width << " " << height << reportEnd();
        output = Image::create(width, height, input->getDataType(), input->getNrOfChannels());
    } else {
        reportInfo() << "New size for volume in image resampler: " << width << " " << height << " " << depth << reportEnd();
        output = Image::create(width, height, depth, input->getDataType(), input->getNrOfChannels());
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
    addOutputData(0, output);
}

void ImageResampler::setInterpolation(bool useInterpolation) {
    mInterpolation = useInterpolation;
    setModified(true);
}

}