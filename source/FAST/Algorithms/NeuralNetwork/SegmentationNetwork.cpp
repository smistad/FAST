#include <FAST/Data/Segmentation.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "SegmentationNetwork.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

void SegmentationNetwork::loadAttributes() {
	if (getBooleanAttribute("heatmap_output")) {
		setHeatmapOutput();
	} else {
		setSegmentationOutput();
	}
	setThreshold(getFloatAttribute("threshold"));
	NeuralNetwork::loadAttributes();
}

SegmentationNetwork::SegmentationNetwork() {
    createInputPort<Image>(0);

    mHeatmapOutput = false;
    createOutputPort<Segmentation>(0);
    mResizeBackToOriginalSize = false;
    mThreshold = 0.5;

    createBooleanAttribute("heatmap_output", "Output heatmap", "Enable heatmap output instead of segmentation", false);
    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", mThreshold);
}

void SegmentationNetwork::setResizeBackToOriginalSize(bool resize) {
    mResizeBackToOriginalSize = resize;
}

void SegmentationNetwork::setHeatmapOutput() {
    mHeatmapOutput = true;
    createOutputPort<Tensor>(0);
}

void SegmentationNetwork::setSegmentationOutput() {
    mHeatmapOutput = false;
    createOutputPort<Segmentation>(0);
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
    return ordering == ImageOrdering::ChannelLast ? x*nrOfClasses + j : x + j*size;
}

void SegmentationNetwork::execute() {

    run();

    mRuntimeManager->startRegularTimer("output_processing");
    Tensor::pointer tensor = m_engine->getOutputNodes().begin()->second.data;
    const auto shape = tensor->getShape();
    if(shape[0] != 1)
        throw Exception("Pixel classifier only support batch size 1 atm");
    TensorAccess::pointer access = tensor->getAccess(ACCESS_READ);
    const int dims = shape.getDimensions();
    int outputHeight = shape[dims-3];
    int outputWidth = shape[dims-2];
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
        outputHeight = shape[dims-2];
        outputWidth = shape[dims-1];
    }
    int outputDepth = 1;
    float* tensorData = access->getRawData();
    if(dims == 5) {
        outputDepth = shape[dims - 4];
    }
    auto ordering = m_engine->getPreferredImageOrdering();

    const int size = outputWidth*outputHeight*outputDepth;
    // TODO reuse some of the output processing in NN
    tensor->deleteDimension(0); // TODO assuming batch size is 1, remove this dimension
    if(mHeatmapOutput) {
        if(ordering == ImageOrdering::ChannelFirst) {
            // Convert to channel last
            const int nrOfClasses = tensor->getShape()[0];
            auto newTensorData = make_uninitialized_unique<float[]>(size*nrOfClasses);
            for(int x = 0; x < size; ++x) {
                for(uchar j = 0; j < nrOfClasses; ++j) {
                    newTensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelLast)] = tensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelFirst)];
                }
            }
            auto newTensor = Tensor::New();
            auto oldShape = tensor->getShape();
            newTensor->create(std::move(newTensorData), TensorShape({oldShape[1], oldShape[2], oldShape[0]}));
            tensor = newTensor;
        }
        tensor->setSpacing(mNewInputSpacing);
        SceneGraph::setParentNode(tensor, mInputImages.begin()->second[0]);
        addOutputData(0, tensor);
    } else {
        auto output = Image::New();
        auto data = make_uninitialized_unique<uchar[]>(size);
        const int nrOfClasses = ordering == ImageOrdering::ChannelFirst ? tensor->getShape()[0] : tensor->getShape()[tensor->getShape().getDimensions()-1];
        for(int x = 0; x < size; ++x) {
            uchar maxClass = 0;
            for(uchar j = 1; j < nrOfClasses; j++) {
                if(tensorData[getPosition(x, nrOfClasses, j, size, ordering)] > mThreshold &&
                        tensorData[getPosition(x, nrOfClasses, j, size, ordering)] > tensorData[getPosition(x, nrOfClasses, maxClass, size, ordering)]) {
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
            DataChannel::pointer port = resizer->getOutputPort();
            resizer->update();

            Image::pointer resizedOutput = port->getNextFrame<Image>();
            addOutputData(0, resizedOutput);
        } else {
            addOutputData(0, output);
        }
    }
    mRuntimeManager->stopRegularTimer("output_processing");
}


void SegmentationNetwork::setThreshold(float threshold) {
    mThreshold = threshold;
}

}