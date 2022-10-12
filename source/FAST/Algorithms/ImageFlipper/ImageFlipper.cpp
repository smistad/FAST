#include "ImageFlipper.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageFlipper::ImageFlipper() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createBooleanAttribute("vertical", "Vertical", "Flip vertical", false);
    createBooleanAttribute("horizontal", "Horizontal", "Flip horizontal", false);
    createBooleanAttribute("depth", "Depth", "Flip depth", false);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageFlipper/ImageFlipper2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageFlipper/ImageFlipper3D.cl", "3D");
}

ImageFlipper::ImageFlipper(bool flipHorizontal, bool flipVertical, bool flipDepth) : ImageFlipper() {
    m_flipHorizontal = flipHorizontal;
    m_flipVertical = flipVertical;
    m_flipDepth = flipDepth;
}

void ImageFlipper::execute() {
    auto input = getInputData<Image>();
	if(!m_flipHorizontal && !m_flipVertical && !m_flipDepth)
		throw Exception("You must select which axes to flip in ImageFlipper");
    auto output = Image::createFromImage(input);
    output->setSpacing(input->getSpacing());
    if(input->getDimensions() == 3) {
        // 3D
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

        auto program = getOpenCLProgram(device, "3D", "-DTYPE=" + getCTypeAsString(input->getDataType()));
        cl::Kernel kernel(program, "flip3D");

        kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get());
        kernel.setArg(2, (char)(m_flipHorizontal ? 1 : 0));
        kernel.setArg(3, (char)(m_flipVertical ? 1 : 0));
        kernel.setArg(4, (char)(m_flipDepth ? 1 : 0));
        kernel.setArg(5, (int)input->getNrOfChannels());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
                );
    } else {
        // 2D
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        auto program = getOpenCLProgram(device, "2D");
        cl::Kernel kernel(program, "flip2D");

        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, (char)(m_flipHorizontal ? 1 : 0));
        kernel.setArg(3, (char)(m_flipVertical ? 1 : 0));

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
                );
    }

    addOutputData(0, output);
}

void ImageFlipper::setFlipHorizontal(bool flip) {
    m_flipHorizontal = flip;
}

void ImageFlipper::setFlipVertical(bool flip) {
    m_flipVertical = flip;
}

void ImageFlipper::setFlipDepth(bool flip) {
    m_flipDepth = flip;
}

void ImageFlipper::loadAttributes() {
    setFlipHorizontal(getBooleanAttribute("horizontal"));
    setFlipVertical(getBooleanAttribute("vertical"));
    setFlipDepth(getBooleanAttribute("depth"));
}


}
