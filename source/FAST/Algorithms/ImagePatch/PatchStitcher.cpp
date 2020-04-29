#include <FAST/Data/Image.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include "PatchStitcher.hpp"

namespace fast {

PatchStitcher::PatchStitcher() {
    createInputPort<DataObject>(0); // Can be Image, Batch or Tensor
    createOutputPort<DataObject>(0); // Can be Image or Tensor

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher3D.cl", "3D");
}

void PatchStitcher::execute() {
    auto patch = getInputData<DataObject>();
    mRuntimeManager->startRegularTimer("stitch patch");
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
    mRuntimeManager->stopRegularTimer("stitch patch");

    if(m_outputImage) {
        addOutputData(0, m_outputImage);
    } else if(m_outputTensor) {
        addOutputData(0, m_outputTensor);
    } else if(m_outputImagePyramid) {
        addOutputData(0, m_outputImagePyramid);
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
        outputTensorData(startY, startX, i) = tensorData(i); // TODO fix
    }
    //outputTensorData(startY, startX, 1) = 1.0f; // TODO remove
}

void PatchStitcher::processImage(SharedPointer<Image> patch) {
    const int fullWidth = std::stoi(patch->getFrameData("original-width"));
    const int fullHeight = std::stoi(patch->getFrameData("original-height"));
    const float patchSpacingX = std::stof(patch->getFrameData("patch-spacing-x"));
    const float patchSpacingY = std::stof(patch->getFrameData("patch-spacing-y"));

    int fullDepth = 1;
    float patchSpacingZ = 1.0f;
    bool is3D = true;
    try {
        fullDepth = std::stoi(patch->getFrameData("original-depth"));
        patchSpacingZ = std::stof(patch->getFrameData("patch-spacing-z"));
    } catch(Exception &e) {
        // If exception: is a 2D image
        is3D = false;
    }

    if(!m_outputImage && !m_outputImagePyramid) {
        // Create output image
        if(is3D) {
			m_outputImage = Image::New();
            m_outputImage->create(fullWidth, fullHeight, fullDepth, patch->getDataType(), patch->getNrOfChannels());
        } else {
            if(fullWidth < 16384 && fullHeight < 16384) {
				m_outputImage = Image::New();
                m_outputImage->create(fullWidth, fullHeight, patch->getDataType(), patch->getNrOfChannels());
            } else {
                // Large image, create image pyramid instead
                m_outputImagePyramid = ImagePyramid::New();
                m_outputImagePyramid->create(fullWidth, fullHeight, patch->getNrOfChannels());
            }
        }
        if(m_outputImage) {
            m_outputImage->fill(0);
            m_outputImage->setSpacing(Vector3f(patchSpacingX, patchSpacingY, patchSpacingZ));
        } else {
            //m_outputImagePyramid->fill(0);
            //m_outputImagePyramid->setSpacing(Vector3f(patchSpacingX, patchSpacingY, patchSpacingZ));
        }
        try {
            auto transformData = split(patch->getFrameData("original-transform"));
            auto T = AffineTransformation::New();
            Affine3f transform;
            for(int i = 0; i < 16; ++i)
                transform.matrix()(i) = std::stof(transformData[i]);
            T->setTransform(transform);
            if(m_outputImage) {
                m_outputImage->getSceneGraphNode()->setTransformation(T);
            } else {
                m_outputImagePyramid->getSceneGraphNode()->setTransformation(T);
            }
        } catch(Exception &e) {

        }
    }

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    if(fullDepth == 1) {
		const int startX = std::stoi(patch->getFrameData("patchid-x")) * std::stoi(patch->getFrameData("patch-width"));
		const int startY = std::stoi(patch->getFrameData("patchid-y")) * std::stoi(patch->getFrameData("patch-height"));
		const int endX = startX + patch->getWidth();
		const int endY = startY + patch->getHeight();
		reportInfo() << "Stitching " << patch->getFrameData("patchid-x") << " " << patch->getFrameData("patchid-y")
			<< reportEnd();
        if(m_outputImage) {
            cl::Program program = getOpenCLProgram(device, "2D");

			auto patchAccess = patch->getOpenCLImageAccess(ACCESS_READ, device);
            auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

            cl::Kernel kernel(program, "applyPatch2D");
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
        } else {
            enableRuntimeMeasurements();
            // Image pyramid, do it on CPU TODO: optimize somehow?
            auto outputAccess = m_outputImagePyramid->getAccess(ACCESS_READ_WRITE);
            auto patchAccess = patch->getImageAccess(ACCESS_READ);
            mRuntimeManager->startRegularTimer("copy patch");
            const int maxY = std::min(endY, fullHeight);
            const int maxX = std::min(endX, fullWidth);
            for(int y = startY; y < maxY; ++y) {
                for(int x = startX; x < maxX; ++x) {
                    outputAccess->setScalarFast(x, y, 0, patchAccess->getScalarFast<uchar>(Vector2i(x - startX, y - startY)));
                }
            }
            mRuntimeManager->stopRegularTimer("copy patch");
            mRuntimeManager->getTiming("copy patch")->print();
        }
    } else {
        // 3D
        const int startX = 0;
        const int startY = 0;
        const int startZ = std::stoi(patch->getFrameData("patch-offset-z"));
        const int endX = startX + patch->getWidth();
        const int endY = startY + patch->getHeight();
        reportInfo() << "Stitching " << startZ << reportEnd();
		auto patchAccess = patch->getOpenCLImageAccess(ACCESS_READ, device);

        if(device->isWritingTo3DTexturesSupported()) {
            auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            cl::Program program = getOpenCLProgram(device, "3D");
            cl::Kernel kernel(program, "applyPatch3D");
            kernel.setArg(0, *patchAccess->get3DImage());
            kernel.setArg(2, startX);
            kernel.setArg(3, startY);
            kernel.setArg(4, startZ);
            kernel.setArg(1, *outputAccess->get3DImage());

            device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(patch->getWidth(), patch->getHeight(), patch->getDepth()),
                cl::NullRange
            );
        } else {
            auto outputAccess = m_outputImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Program program = getOpenCLProgram(device, "3D", "-DTYPE=" + getCTypeAsString(m_outputImage->getDataType()));
            cl::Kernel kernel(program, "applyPatch3D");
            kernel.setArg(0, *patchAccess->get3DImage());
            kernel.setArg(1, *outputAccess->get());
            kernel.setArg(2, startX);
            kernel.setArg(3, startY);
            kernel.setArg(4, startZ);
            kernel.setArg(5, fullWidth);
            kernel.setArg(6, fullHeight);
            kernel.setArg(7, m_outputImage->getNrOfChannels());

            device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(patch->getWidth(), patch->getHeight(), patch->getDepth()),
                cl::NullRange
            );
        }



    }
}


}