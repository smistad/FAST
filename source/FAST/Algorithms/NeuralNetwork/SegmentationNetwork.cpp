#include <FAST/Data/Segmentation.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "SegmentationNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "TensorToSegmentation.hpp"

namespace fast {

void SegmentationNetwork::loadAttributes() {
	if (getBooleanAttribute("heatmap-output")) {
		setHeatmapOutput();
	} else {
		setSegmentationOutput();
	}
	setThreshold(getFloatAttribute("threshold"));
	NeuralNetwork::loadAttributes();
}

SegmentationNetwork::SegmentationNetwork() {
    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0);

    m_tensorToSegmentation = TensorToSegmentation::New();
    mHeatmapOutput = false;
    createBooleanAttribute("heatmap-output", "Output heatmap", "Enable heatmap output instead of segmentation", false);
    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", 0.5f);
}

void SegmentationNetwork::setHeatmapOutput() {
    mHeatmapOutput = true;
    createOutputPort<Tensor>(0);
}

void SegmentationNetwork::setSegmentationOutput() {
    mHeatmapOutput = false;
    createOutputPort<Segmentation>(0);
}

void SegmentationNetwork::setResizeBackToOriginalSize(bool resize) {
    m_resizeBackToOriginalSize = resize;
}

void SegmentationNetwork::execute() {
    run();

    auto data = m_processedOutputData[0];
    if(mHeatmapOutput) {
        addOutputData(0, data);
    } else {
        m_tensorToSegmentation->setInputData(data);
        auto image = m_tensorToSegmentation->updateAndGetOutputData<Image>();
        if(m_resizeBackToOriginalSize) {
            auto resizer = ImageResizer::New();
            resizer->setInputData(image);
            resizer->setSize(mInputImages.begin()->second[0]->getSize().cast<int>());
            resizer->setInterpolation(false);
            image = resizer->updateAndGetOutputData<Image>();
        }
        addOutputData(0, image);
    }
    mRuntimeManager->stopRegularTimer("output_processing");
}


void SegmentationNetwork::setThreshold(float threshold) {
    m_tensorToSegmentation->setThreshold(threshold);
}

}