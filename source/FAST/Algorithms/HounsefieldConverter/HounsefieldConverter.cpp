#include "HounsefieldConverter.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

HounsefieldConverter::HounsefieldConverter() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/HounsefieldConverter/HounsefieldConverter.cl");
}

Image::pointer HounsefieldConverter::convertToHU(Image::pointer image) {
	auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
	auto program = getOpenCLProgram(device);

	auto input = image->getOpenCLImageAccess(ACCESS_READ, device);
	auto newImage = Image::create(image->getSize(), TYPE_INT16, 1);
	newImage->setSpacing(image->getSpacing());
	SceneGraph::setParentNode(newImage, image);

	cl::Kernel kernel(program, "convertToHU");

	kernel.setArg(0, *input->get3DImage());
	if(device->isWritingTo3DTexturesSupported()) {
		OpenCLImageAccess::pointer output = newImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(1, *output->get3DImage());
	} else {
		OpenCLBufferAccess::pointer output = newImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(1, *output->get());
	}

	device->getCommandQueue().enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(image->getWidth(), image->getHeight(), image->getDepth()),
			cl::NullRange
    );

	return newImage;
}

void HounsefieldConverter::execute() {
	Image::pointer image = getInputData<Image>();

	// Convert to signed HU if unsigned
	if(image->getDataType() == TYPE_UINT16) {
		image = convertToHU(image);
	}
	if(image->getDataType() != TYPE_INT16) {
		throw Exception("Input image to hounsefield converter must be of data type INT16 or UINT16");
	}

	addOutputData(0, image);
}

}