#include "ZeroMeanUnitVariance.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ZeroMeanUnitVariance::ZeroMeanUnitVariance() {
	createInputPort(0);
	createOutputPort(0);

	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/IntensityNormalization/ZeroMeanUnitVariance.cl");
}

void ZeroMeanUnitVariance::execute() {
	auto input = getInputData<Image>();
	float average = input->calculateAverageIntensity();
	float standardDeviation = input->calculateStandardDeviationIntensity();

	auto output = Image::create(input->getSize(), TYPE_FLOAT, input->getNrOfChannels());
	output->setSpacing(input->getSpacing());
	SceneGraph::setParentNode(output, input);

	auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
	cl::Kernel kernel;
	cl::NDRange globalSize;
	if(input->getDimensions() == 2) {
        globalSize = cl::NDRange(input->getWidth(), input->getHeight());
		kernel = cl::Kernel(getOpenCLProgram(device), "normalize2D");
		auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
		auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(0, *inputAccess->get2DImage());
		kernel.setArg(1, *outputAccess->get2DImage());
	} else {
		// 3D
        globalSize = cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth());
		kernel = cl::Kernel(getOpenCLProgram(device),"normalize3D");
		auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);

		kernel.setArg(0, *inputAccess->get3DImage());
	   if(device->isWritingTo3DTexturesSupported()) {
            auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get3DImage()));
        } else {
            auto outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get()));
            kernel.setArg(4, output->getNrOfChannels());
        }
	}
	kernel.setArg(2, average);
	kernel.setArg(3, standardDeviation);
	
    cl::CommandQueue queue = device->getCommandQueue();

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            globalSize,
            cl::NullRange
    );


	addOutputData(0, output);
}

}
