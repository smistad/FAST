#include <FAST/Data/Segmentation.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "PixelClassifier.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

PixelClassifier::PixelClassifier() {
    createInputPort<Image>(0);

    mNrOfClasses = -1;
    mHeatmapOutput = false;
    mResizeBackToOriginalSize = false;
    mThreshold = 0.5;

    createIntegerAttribute("classes", "Classes", "Number of possible classes for each pixel", 2);
    createBooleanAttribute("heatmap_output", "Output heatmap", "Enable heatmap output instead of segmentation", false);
}

void PixelClassifier::setResizeBackToOriginalSize(bool resize) {
    mResizeBackToOriginalSize = resize;
}

void PixelClassifier::setHeatmapOutput() {
    mHeatmapOutput = true;
}

void PixelClassifier::setSegmentationOutput() {
    mHeatmapOutput = false;
}

void PixelClassifier::setNrOfClasses(uint classes) {
    mNrOfClasses = classes;
    if(mHeatmapOutput) {
        for (int i = 0; i < mNrOfClasses; i++) {
            createOutputPort<Image>(i);
        }
    } else {
        createOutputPort<Image>(0);
    }
}

void PixelClassifier::execute() {
    mRuntimeManager->enable();
    mRuntimeManager->startRegularTimer("pixel_classifier");
    if(mNrOfClasses <= 0)
        throw Exception("You must set the nr of classes to pixel classification.");

    auto input = processInputData();
    auto result = executeNetwork(input);
    Tensor::pointer tensor = result[0].second;
    const auto shape = tensor->getShape();
    TensorAccess::pointer access = tensor->getAccess(ACCESS_READ);
    reportInfo() << "Processing output of NN" << reportEnd();
    auto tensor_mapped = access->getData<4>();
    reportInfo() << "Got eigen tensor" << reportEnd();
    const int dims = shape.getDimensions();
    int outputHeight = shape[dims-3];
    int outputWidth = shape[dims-2];
    int outputDepth = 1;
    if(dims == 5)
        outputDepth = shape[dims-4];

    if(mHeatmapOutput) {
        for(int j = 0; j < mNrOfClasses; j++) {
            // Check if output for this class has been requested
            if (mOutputConnections[j].empty())
                continue;
            // TODO fix 3D support here
            if(outputDepth != 1)
                throw NotImplementedException();
            auto data = make_uninitialized_unique<float[]>(outputWidth * outputHeight * outputDepth);
            if (mHorizontalImageFlipping) {
                for (int x = 0; x < outputWidth; ++x) {
                    for (int y = 0; y < outputHeight; ++y) {
                        data[x + y * outputWidth] = tensor_mapped(0, y, outputWidth - x - 1, j);// > threshold ? j : 0;
                    }
                }
            } else {
                for (int x = 0; x < outputWidth; ++x) {
                    for (int y = 0; y < outputHeight; ++y) {
                        data[x + y * outputWidth] = tensor_mapped(0, y, x, j);// > threshold ? j : 0;
                    }
                }
            }
            Image::pointer output = Image::New();
            output->create(outputWidth, outputHeight, TYPE_FLOAT, 1, std::move(data));

            output->setSpacing(mNewInputSpacing);
            SceneGraph::setParentNode(output, mInputImages.begin()->second[0]);
            if(mResizeBackToOriginalSize) {
                ImageResizer::pointer resizer = ImageResizer::New();
                resizer->setInputData(output);
                resizer->setSize(mInputImages.begin()->second[0]->getSize().cast<int>());
                resizer->setPreserveAspectRatio(mPreserveAspectRatio);
                DataPort::pointer port = resizer->getOutputPort();
                resizer->update(0);

                Image::pointer resizedOutput = port->getNextFrame<Image>();
                addOutputData(j, resizedOutput);
            } else {
                addOutputData(j, output);
            }
        }
    } else {
        reportInfo() << "Converting data" << reportEnd();
        Image::pointer output = Image::New();
        auto data = make_uninitialized_unique<uchar[]>(outputWidth * outputHeight * outputDepth);
        for(int x = 0; x < outputWidth*outputHeight*outputDepth; ++x) {
            data[x] = 0;
            int maxClass = 0;
            for(int j = 1; j < mNrOfClasses; j++) {
                if(tensor_mapped(x*mNrOfClasses + j) > mThreshold && tensor_mapped(x*mNrOfClasses + j) > tensor_mapped(x*mNrOfClasses + maxClass)) {
                    data[x] = j;
                    maxClass = j;
                }
            }
        }
        if(outputDepth == 1) {
            output->create(outputWidth, outputHeight, TYPE_UINT8, 1, std::move(data));
        } else {
            output->create(outputWidth, outputHeight, outputDepth, TYPE_UINT8, 1, std::move(data));
        }
        output->setSpacing(mNewInputSpacing);
        SceneGraph::setParentNode(output, mInputImages.begin()->second[0]);
        if(mResizeBackToOriginalSize) {
            ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setInputData(output);
            resizer->setSize(mInputImages.begin()->second[0]->getSize().cast<int>());
            resizer->setPreserveAspectRatio(mPreserveAspectRatio);
            DataPort::pointer port = resizer->getOutputPort();
            resizer->update(0);

            Image::pointer resizedOutput = port->getNextFrame<Image>();
            addOutputData(0, resizedOutput);
        } else {
            addOutputData(0, output);
        }
    }

    mRuntimeManager->stopRegularTimer("pixel_classifier");
}

void PixelClassifier::loadAttributes() {
    if(getBooleanAttribute("heatmap_output")) {
        setHeatmapOutput();
    } else {
        setSegmentationOutput();
    }
    setNrOfClasses(getIntegerAttribute("classes"));
    NeuralNetwork::loadAttributes();
}

void PixelClassifier::setThreshold(float threshold) {
    mThreshold = threshold;
}

}