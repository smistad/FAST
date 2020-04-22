#include <FAST/Data/ImagePyramid.hpp>
#include "TissueSegmentation.hpp"
#include <FAST/Algorithms/Morphology/Dilation.hpp>
#include <FAST/Algorithms/Morphology/Erosion.hpp>
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>

namespace fast {

TissueSegmentation::TissueSegmentation() {
    createInputPort<ImagePyramid>(0);
    createOutputPort<Segmentation>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/TissueSegmentation/TissueSegmentation.cl");
}

void TissueSegmentation::execute() {
    auto wsi = getInputData<ImagePyramid>();
    auto access = wsi->getAccess(ACCESS_READ);
    auto input = access->getLevelAsImage(wsi->getNrOfLevels()-1);

    auto output = Segmentation::New();
    output->createFromImage(input);

    {
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto program = getOpenCLProgram(device);
        cl::Kernel kernel(program, "segment");

        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
        device->getCommandQueue().finish();
    }

    auto erosion = Erosion::New();
    erosion->setInputData(output);
    erosion->setStructuringElementSize(9);

    auto dilation = Dilation::New();
    dilation->setInputConnection(erosion->getOutputPort());
    dilation->setStructuringElementSize(17);

    auto newOutput = dilation->updateAndGetOutputData<Image>();
    addOutputData(0, newOutput);

}

}