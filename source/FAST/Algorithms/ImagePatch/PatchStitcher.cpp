#include <FAST/Data/Image.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include "PatchStitcher.hpp"

namespace fast {

PatchStitcher::PatchStitcher() {
    createInputPort<DataObject>(0); // Can be Image, Batch or Tensor
    createOutputPort<DataObject>(0); // Can be Image or Tensor

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher.cl");
}

void PatchStitcher::execute() {
    auto patch = getInputData<DataObject>();
    auto tensorPatch = std::dynamic_pointer_cast<Tensor>(patch);
    auto imagePatch = std::dynamic_pointer_cast<Image>(patch);
    auto batchOfPatches = std::dynamic_pointer_cast<Batch>(patch);
    if(batchOfPatches) {
       auto access = batchOfPatches->getAccess(ACCESS_READ);
       auto list = access->getData();
       if(list.isTensors()) {
           for(auto&& tensor : list.getTensors())
               processTensor(tensor);
       }
        if(list.isImages()) {
            for(auto&& image : list.getImages())
                processImage(image);
        }
    } else {
        // Single
        if(tensorPatch) {
            processTensor(tensorPatch);
        } else if(imagePatch) {
            processImage(imagePatch);
        }
    }

    if(m_outputImage) {
        addOutputData(0, m_outputImage);
    } else if (m_outputTensor) {
        addOutputData(0, m_outputTensor);
    } else {
        throw Exception("Unexpected event in PatchStitcher");
    }
}

void PatchStitcher::processTensor(SharedPointer<Tensor> patch) {
    const int fullWidth = std::stoi(patch->getFrameData("original-width"));
    const int fullHeight = std::stoi(patch->getFrameData("original-height"));

    const int patchWidth = std::stoi(patch->getFrameData("patch-width"));
    const int patchHeight = std::stoi(patch->getFrameData("patch-height"));

    const float patchSpacingX = std::stof(patch->getFrameData("patch-spacing-x"));
    const float patchSpacingY = std::stof(patch->getFrameData("patch-spacing-y"));

    auto shape = patch->getShape();
    std::cout << "shape: " << shape.toString() << std::endl;
    if(shape.getDimensions() != 1) {
        throw Exception("Can only handle 1D tensors atm");
    }
    const int channels = shape[0];

    if(!m_outputTensor) {
        // Create output tensor
        m_outputTensor = Tensor::New();
        TensorShape fullShape({(int)std::ceil((float)fullHeight / patchHeight), (int)std::ceil((float)fullWidth / patchWidth), channels});
        auto initializedData = std::make_unique<float[]>(fullShape.getTotalSize());
        m_outputTensor->create(std::move(initializedData), fullShape);
        m_outputTensor->setSpacing(Vector3f(patchHeight*patchSpacingY, patchWidth*patchSpacingX, 1.0f));
    }
    reportInfo() << "Stitching " << patch->getFrameData("patchid-x") << " " << patch->getFrameData("patchid-y") << reportEnd();
    reportInfo() << "Stitching data" << patch->getFrameData("patch-spacing-x") << " " << patch->getFrameData("patch-spacing-y") << reportEnd();

    const int startX = std::stoi(patch->getFrameData("patchid-x"));
    const int startY = std::stoi(patch->getFrameData("patchid-y"));

    auto inputAccess = patch->getAccess(ACCESS_READ);
    auto tensorData = inputAccess->getData<1>();
    auto outputAccess = m_outputTensor->getAccess(ACCESS_READ_WRITE);
    auto outputTensorData = outputAccess->getData<3>();

    for(int i = 0; i < channels; ++i) {
        outputTensorData(startY, startX, i) = 0;//tensorData(i); // TODO fix
    }
    outputTensorData(startY, startX, 1) = 1.0f; // TODO remove
}

void PatchStitcher::processImage(SharedPointer<Image> patch) {
    const int fullWidth = std::stoi(patch->getFrameData("original-width"));
    const int fullHeight = std::stoi(patch->getFrameData("original-height"));

    if(!m_outputImage) {
        // Create output image
        m_outputImage = Image::New();
        m_outputImage->create(fullWidth, fullHeight, patch->getDataType(), patch->getNrOfChannels());
        m_outputImage->fill(0);
    }

    const int startX = std::stoi(patch->getFrameData("patchid-x")) * std::stoi(patch->getFrameData("patch-width"));
    const int startY = std::stoi(patch->getFrameData("patchid-y")) * std::stoi(patch->getFrameData("patch-height"));
    const int endX = startX + patch->getWidth();
    const int endY = startY + patch->getHeight();
    reportInfo() << "Stitching " << patch->getFrameData("patchid-x") << " " << patch->getFrameData("patchid-y") << reportEnd();

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);

    auto patchAccess = patch->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    cl::Kernel kernel(program, "applyPatch");
    kernel.setArg(0, *patchAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, startX);
    kernel.setArg(3, startY);
    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(patch->getWidth(), patch->getHeight()),
            cl::NullRange
    );
}


}