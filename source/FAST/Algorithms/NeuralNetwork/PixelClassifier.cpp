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

/**
 * Calculate array position based on image ordering
 * @param x
 * @param nrOfClasses
 * @param j
 * @param size
 * @param ordering
 * @return
 */
inline int getPosition(int x, int nrOfClasses, int j, int size, ImageOrdering ordering) {
    return ordering == ImageOrdering::HWC ? x*nrOfClasses + j : x + j*size;
}

void PixelClassifier::execute() {
    mRuntimeManager->enable();
    mRuntimeManager->startRegularTimer("pixel_classifier");
    if(mNrOfClasses <= 0)
        throw Exception("You must set the nr of classes to pixel classification.");

    run();

    Tensor::pointer tensor = m_engine->getOutputNodes().begin()->second.data;
    const auto shape = tensor->getShape();
    TensorAccess::pointer access = tensor->getAccess(ACCESS_READ);
    const int dims = shape.getDimensions();
    int outputHeight = shape[dims-3];
    int outputWidth = shape[dims-2];
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::CHW) {
        outputHeight = shape[dims-2];
        outputWidth = shape[dims-1];
    }
    int outputDepth = 1;
    float* tensorData;
    if(dims == 5) {
        outputDepth = shape[dims - 4];
        tensorData = access->getData<5>().data();
    } else {
        tensorData = access->getData<4>().data();
    }
    auto ordering = m_engine->getPreferredImageOrdering();

    const int size = outputWidth*outputHeight*outputDepth;
    if(mHeatmapOutput) {
        for(int j = 0; j < mNrOfClasses; j++) {
            // Check if output for this class has been requested
            if (mOutputConnections[j].empty())
                continue;
            auto data = make_uninitialized_unique<float[]>(size);
            for(int x = 0; x < size; ++x) {
                data[x] = tensorData[getPosition(x, mNrOfClasses, j, size, ordering)];
            }
            Image::pointer output = Image::New();
            if(outputDepth == 1) {
                output->create(outputWidth, outputHeight, TYPE_FLOAT, 1, std::move(data));
            } else {
                output->create(outputWidth, outputHeight, outputDepth, TYPE_FLOAT, 1, std::move(data));
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
                addOutputData(j, resizedOutput);
            } else {
                addOutputData(j, output);
            }
        }
    } else {
        Image::pointer output = Image::New();
        auto data = make_uninitialized_unique<uchar[]>(size);
        for(int x = 0; x < size; ++x) {
            uchar maxClass = 0;
            for(uchar j = 1; j < mNrOfClasses; j++) {
                if(tensorData[getPosition(x, mNrOfClasses, j, size, ordering)] > mThreshold &&
                        tensorData[getPosition(x, mNrOfClasses, j, size, ordering)] > tensorData[getPosition(x, mNrOfClasses, maxClass, size, ordering)]) {
                    maxClass = j;
                }
            }
            data[x] = maxClass;
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
            resizer->setInterpolation(false);
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