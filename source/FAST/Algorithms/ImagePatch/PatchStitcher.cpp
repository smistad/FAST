#include <FAST/Data/Image.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "PatchStitcher.hpp"

namespace fast {

PatchStitcher::PatchStitcher(bool patchesAreCropped) {
    createInputPort<DataObject>(0); // Can be Image, Batch or Tensor
    createOutputPort<DataObject>(0); // Can be Image or Tensor

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher3D.cl", "3D");
    createBooleanAttribute("patches-are-cropped", "Patches are cropped", "Indicate whether incomming patches are already cropped or not.", false);
    setPatchesAreCropped(patchesAreCropped);
}

void PatchStitcher::loadAttributes() {
    setPatchesAreCropped(getBooleanAttribute("patches-are-cropped"));
}

void PatchStitcher::execute() {
    auto patch = getInputData<DataObject>();
    mRuntimeManager->startRegularTimer("stitch patch");
    auto tensorPatch = std::dynamic_pointer_cast<Tensor>(patch);
    auto imagePatch = std::dynamic_pointer_cast<Image>(patch);
    auto batchOfPatches = std::dynamic_pointer_cast<Batch>(patch);
    if(batchOfPatches) {
       auto list = batchOfPatches->get();
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

void PatchStitcher::processTensor(std::shared_ptr<Tensor> patch) {
    const int fullWidth = std::stoi(patch->getFrameData("original-width"));
    const int fullHeight = std::stoi(patch->getFrameData("original-height"));

    const int patchWidth = std::stoi(patch->getFrameData("patch-width"))- 2*std::stoi(patch->getFrameData("patch-overlap-x"));;
    const int patchHeight = std::stoi(patch->getFrameData("patch-height")) - 2*std::stoi(patch->getFrameData("patch-overlap-y"));;

    const float patchSpacingX = std::stof(patch->getFrameData("patch-spacing-x"));
    const float patchSpacingY = std::stof(patch->getFrameData("patch-spacing-y"));

    auto shape = patch->getShape();
    if(shape.getDimensions() != 1) {
        throw Exception("Can only handle 1D tensors atm");
    }
    const int channels = shape[0];

    if(!m_outputTensor) {
        // Create output tensor
        TensorShape fullShape({(int)std::ceil((float)fullHeight / patchHeight), (int)std::ceil((float)fullWidth / patchWidth), channels});
        auto initializedData = std::make_unique<float[]>(fullShape.getTotalSize());
        m_outputTensor = Tensor::create(std::move(initializedData), fullShape);
        // TODO Use Y-X or X-Y ordering on spacing here? Changes will influence other objects
        m_outputTensor->setSpacing(Vector3f(patchHeight*patchSpacingY, patchWidth*patchSpacingX, 1.0f));
    }
    reportInfo() << "Stitching " << patch->getFrameData("patchid-x") << " " << patch->getFrameData("patchid-y") << reportEnd();
    reportInfo() << "Stitching data with spacing " << patch->getFrameData("patch-spacing-x") << " " << patch->getFrameData("patch-spacing-y") << reportEnd();

    const int startX = std::stoi(patch->getFrameData("patchid-x"));
    const int startY = std::stoi(patch->getFrameData("patchid-y"));

    auto inputAccess = patch->getAccess(ACCESS_READ);
    auto tensorData = inputAccess->getData<1>();
    auto outputAccess = m_outputTensor->getAccess(ACCESS_READ_WRITE);
    auto outputTensorData = outputAccess->getData<3>();

    for(int i = 0; i < channels; ++i) {
        outputTensorData(startY, startX, i) = tensorData(i);
    }
}

void PatchStitcher::processImage(std::shared_ptr<Image> patch) {
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
        if(fullDepth == 1)
            is3D = false;
    } catch(Exception &e) {
        // If exception: is a 2D image
        is3D = false;
    }

    if(!m_outputImage && !m_outputImagePyramid) {
        // Create output image
        if(is3D) {
			m_outputImage = Image::create(fullWidth, fullHeight, fullDepth, patch->getDataType(), patch->getNrOfChannels());
        } else {
            if(fullWidth < 8192 && fullHeight < 8192) {
                reportInfo() << "Patch stitcher creating image with size " << fullWidth << " " << fullHeight << reportEnd();
				m_outputImage = Image::create(fullWidth, fullHeight, patch->getDataType(), patch->getNrOfChannels());
            } else {
                // Large image, create image pyramid instead
                int patchWidth = std::stoi(patch->getFrameData("patch-width")) - 2*std::stoi(patch->getFrameData("patch-overlap-x"));
                int patchHeight = std::stoi(patch->getFrameData("patch-height")) - 2*std::stoi(patch->getFrameData("patch-overlap-y"));
                m_outputImagePyramid = ImagePyramid::create(fullWidth, fullHeight, patch->getNrOfChannels(), patchWidth, patchHeight);
                reportInfo() << "Patch stitcher creating image PYRAMID with size " << fullWidth << " " << fullHeight << ", patch size: " <<
                    patchWidth << " " << patchHeight << " Levels: " << m_outputImagePyramid->getNrOfLevels() << reportEnd();
            }
        }
        if(m_outputImage) {
            m_outputImage->fill(0);
            m_outputImage->setSpacing(Vector3f(patchSpacingX, patchSpacingY, patchSpacingZ));
        } else {
            //m_outputImagePyramid->fill(0);
            m_outputImagePyramid->setSpacing(Vector3f(patchSpacingX, patchSpacingY, patchSpacingZ));
        }
        try {
            auto transformData = split(patch->getFrameData("original-transform"));
            auto T = Transform::create();
            Affine3f transform;
            for(int i = 0; i < 16; ++i)
                transform.matrix()(i) = std::stof(transformData[i]);
            T->set(transform);
            if(m_outputImage) {
                m_outputImage->getSceneGraphNode()->setTransform(T);
            } else {
                m_outputImagePyramid->getSceneGraphNode()->setTransform(T);
            }
        } catch(Exception &e) {

        }
    }

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    if(fullDepth == 1) {
        // 2D
		reportInfo() << "Stitching 2D data " << patch->getFrameData("patchid-x") << " " << patch->getFrameData("patchid-y")
			<< reportEnd();

        const int patchOverlapX = std::stoi(patch->getFrameData("patch-overlap-x"));
        const int patchOverlapY = std::stoi(patch->getFrameData("patch-overlap-y"));
        // Calculate offset. If this calculation is incorrect. Update in ImagePyramidPatchExporter as well.
        // Position of where to insert the (cropped) patch
        const int startX = std::stoi(patch->getFrameData("patchid-x")) * (std::stoi(patch->getFrameData("patch-width")) - patchOverlapX*2); // TODO + overlap to compensate for start offset
        const int startY = std::stoi(patch->getFrameData("patchid-y")) * (std::stoi(patch->getFrameData("patch-height")) - patchOverlapY*2);
        if(m_outputImage) {
            // 2D image
            cl::Program program = getOpenCLProgram(device, "2D");

			auto patchAccess = patch->getOpenCLImageAccess(ACCESS_READ, device);
            auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

            cl::Kernel kernel(program, "applyPatch2D");
            kernel.setArg(0, *patchAccess->get2DImage());
            kernel.setArg(1, *outputAccess->get2DImage());
            kernel.setArg(2, startX);
            kernel.setArg(3, startY);
            kernel.setArg(4, patchOverlapX);
            kernel.setArg(5, patchOverlapY);
            device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(
                        std::min(patch->getWidth(), m_outputImage->getWidth()-startX),
                        std::min(patch->getHeight(), m_outputImage->getHeight()-startY)
                    ),
                cl::NullRange
            );
        } else {
            // 2D image pyramid
            if(patch->getWidth() <= patchOverlapX*2 || patch->getHeight() <= patchOverlapY*2) // Image to small..
                return;
            auto outputAccess = m_outputImagePyramid->getAccess(ACCESS_READ_WRITE);
            mRuntimeManager->startRegularTimer("copy patch");
            if(!m_patchesAreCropped) {
                if(patchOverlapX > 0 || patchOverlapY > 0) {
                    patch = patch->crop(
                            Vector2i(patchOverlapX, patchOverlapY),
                            Vector2i(patch->getWidth() - patchOverlapX * 2, patch->getHeight() - patchOverlapY * 2)
                            );
                }
            }
            if(patch->getWidth() != m_outputImagePyramid->getLevelTileWidth(0) || patch->getHeight() != m_outputImagePyramid->getLevelTileHeight(0)) {
                // The patch size has been modified due to TIFF limitation of multiplum of 16
                int diffX = patch->getWidth() - m_outputImagePyramid->getLevelTileWidth(0);
                int diffY = patch->getHeight() - m_outputImagePyramid->getLevelTileHeight(0);
                patch = patch->crop(Vector2i(diffX/2, diffY/2), Vector2i(patch->getWidth()-diffX, patch->getHeight()-diffY));
            }
            outputAccess->setPatch(0, startX, startY, patch);
            mRuntimeManager->stopRegularTimer("copy patch");
        }
    } else {
        // 3D
        // TODO overlap not implemented for 3D
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
            cl::Kernel kernel;
            cl::NDRange size;
            if(patch->getDimensions() == 2) {
                kernel = cl::Kernel(program, "applyPatch2Dto3D");
                size = cl::NDRange(patch->getWidth(), patch->getHeight());
                kernel.setArg(0, *patchAccess->get3DImage());
            } else {
                kernel = cl::Kernel(program, "applyPatch3D");
                size =
                kernel.setArg(0, *patchAccess->get2DImage());
            }
            kernel.setArg(2, startX);
            kernel.setArg(3, startY);
            kernel.setArg(4, startZ);
            kernel.setArg(1, *outputAccess->get3DImage());

            device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                size,
                cl::NullRange
            );
        } else {
            auto outputAccess = m_outputImage->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Program program = getOpenCLProgram(device, "3D", "-DTYPE=" + getCTypeAsString(m_outputImage->getDataType()));
            cl::Kernel kernel;
            cl::NDRange size;
            if(patch->getDimensions() == 2) {
                kernel = cl::Kernel(program, "applyPatch2Dto3D");
                size = cl::NDRange(patch->getWidth(), patch->getHeight());
                kernel.setArg(0, *patchAccess->get3DImage());
            } else {
                kernel = cl::Kernel(program, "applyPatch3D");
                size = cl::NDRange(patch->getWidth(), patch->getHeight(), patch->getDepth());
                kernel.setArg(0, *patchAccess->get2DImage());
            }
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
                size,
                cl::NullRange
            );
        }



    }
}

void PatchStitcher::setPatchesAreCropped(bool cropped) {
    m_patchesAreCropped = cropped;
    setModified(true);
}

bool PatchStitcher::getPatchesAreCropped() const {
    return m_patchesAreCropped;
}


}