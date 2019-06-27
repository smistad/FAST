#include <FAST/Data/Image.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include "PatchStitcher.hpp"

namespace fast {

PatchStitcher::PatchStitcher() {
    createInputPort<DataObject>(0); // Can be Image, Batch or Tensor
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher.cl");
}

void PatchStitcher::execute() {
    auto patch = getInputData<DataObject>();
    auto tensorPatch = std::dynamic_pointer_cast<Tensor>(patch);
    auto imagePatch = std::dynamic_pointer_cast<Image>(patch);
    auto batchOfPatches = std::dynamic_pointer_cast<Batch>(patch);
    if(tensorPatch) {
        const int fullWidth = std::stoi(patch->getMetadata("original-width"));
        const int fullHeight = std::stoi(patch->getMetadata("original-height"));

        const int patchWidth = std::stoi(patch->getMetadata("patch-width"));
        const int patchHeight = std::stoi(patch->getMetadata("patch-height"));

        const float patchSpacingX = std::stof(patch->getMetadata("patch-spacing-x"));
        const float patchSpacingY = std::stof(patch->getMetadata("patch-spacing-y"));

        auto shape = tensorPatch->getShape();
        if(shape.getDimensions() != 2) {
            throw Exception("Can only handle 1D tensors atm");
        }

        if(!m_outputImage) {
            // Create output image
            m_outputImage = Image::New();
            m_outputImage->create(std::ceil((float)fullWidth / patchWidth), std::ceil((float)fullHeight / patchHeight), TYPE_FLOAT, 1);
            m_outputImage->fill(0);
            m_outputImage->setSpacing(Vector3f(patchWidth*patchSpacingX, patchHeight*patchSpacingY, 1.0f));
        }

        const int startX = std::stoi(patch->getMetadata("patchid-x"));
        const int startY = std::stoi(patch->getMetadata("patchid-y"));

        auto inputAccess = tensorPatch->getAccess(ACCESS_READ);
        auto tensorData = inputAccess->getData<2>();
        auto outputAccess = m_outputImage->getImageAccess(ACCESS_READ_WRITE);

        int channelSelected = 1;
        outputAccess->setScalar(Vector2i(startX, startY), tensorData(0, channelSelected));

    } else if(imagePatch) {
        const int fullWidth = std::stoi(patch->getMetadata("original-width"));
        const int fullHeight = std::stoi(patch->getMetadata("original-height"));

        if(!m_outputImage) {
            // Create output image
            m_outputImage = Image::New();
            m_outputImage->create(fullWidth, fullHeight, imagePatch->getDataType(), imagePatch->getNrOfChannels());
            m_outputImage->fill(0);
        }

        const int startX = std::stoi(patch->getMetadata("patchid-x")) * std::stoi(patch->getMetadata("patch-width"));
        const int startY = std::stoi(patch->getMetadata("patchid-y")) * std::stoi(patch->getMetadata("patch-height"));
        const int endX = startX + imagePatch->getWidth();
        const int endY = startY + imagePatch->getHeight();
        reportInfo() << "Stitching " << patch->getMetadata("patchid-x") << " " << patch->getMetadata("patchid-y")
                     << reportEnd();

        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        cl::Program program = getOpenCLProgram(device);

        auto patchAccess = imagePatch->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        cl::Kernel kernel(program, "applyPatch");
        kernel.setArg(0, *patchAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, startX);
        kernel.setArg(3, startY);
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(imagePatch->getWidth(), imagePatch->getHeight()),
                cl::NullRange
        );
    }

    addOutputData(0, m_outputImage);
}


}