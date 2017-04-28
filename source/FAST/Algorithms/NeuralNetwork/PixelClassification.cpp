#include <FAST/Data/Segmentation.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "PixelClassification.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

PixelClassification::PixelClassification() {
    createInputPort<Image>(0);

    mNrOfClasses = -1;
    mHeatmapOutput = false;

    createIntegerAttribute("classes", "Classes", "Number of possible classes for each pixel", 2);
    createBooleanAttribute("heatmap_output", "Output heatmap", "Enable heatmap output instead of segmentation", false);
}

void PixelClassification::setHeatmapOutput() {
    mHeatmapOutput = true;
}

void PixelClassification::setSegmentationOutput() {
    mHeatmapOutput = false;
}

void PixelClassification::setNrOfClasses(uint classes) {
    mNrOfClasses = classes;
    for(int i = 0; i < mNrOfClasses; i++) {
        createOutputPort<Image>(i, OUTPUT_DEPENDS_ON_INPUT, 0);
    }
}

void PixelClassification::execute() {
    if(mNrOfClasses <= 0) {
        throw Exception("You must set the nr of classes to pixel classification.");
    }
    NeuralNetwork::execute();

    tensorflow::Tensor tensor = getNetworkOutput();
    Eigen::Tensor<float, 4, 1> tensor_mapped = tensor.tensor<float, 4>();
    int outputHeight = tensor_mapped.dimension(1);
    int outputWidth = tensor_mapped.dimension(2);

    // TODO assuming only one input image for now
    // For each class
    for(int j = 0; j < mNrOfClasses; j++) {
        Image::pointer output = Image::New();
        if(mHeatmapOutput) {
            float *data = new float[outputWidth * outputHeight];
            for (int x = 0; x < outputWidth; ++x) {
                for (int y = 0; y < outputHeight; ++y) {
                    data[x + y * outputWidth] = tensor_mapped(0, y, x, j);// > threshold ? j : 0;
                }
            }
            output->create(outputWidth, outputHeight, TYPE_FLOAT, 1, data);
            delete[] data;
        } else {
            uchar *data = new uchar[outputWidth * outputHeight];
            const float threshold = 0.75;
            for (int x = 0; x < outputWidth; ++x) {
                for (int y = 0; y < outputHeight; ++y) {
                    data[x + y * outputWidth] = tensor_mapped(0, y, x, j) > threshold ? j : 0;
                }
            }
            output->create(outputWidth, outputHeight, TYPE_UINT8, 1, data);
            delete[] data;
        }
        ImageResizer::pointer resizer = ImageResizer::New();
        resizer->setInputData(output);
        resizer->setWidth(mImage->getWidth());
        resizer->setHeight(mImage->getHeight());
        resizer->update();

        Image::pointer resizedOutput = resizer->getOutputData<Image>();
        resizedOutput->setSpacing(mImage->getSpacing());
        setStaticOutputData<Image>(j, resizedOutput);
    }

}

void PixelClassification::loadAttributes() {
    NeuralNetwork::loadAttributes();
    setNrOfClasses(getIntegerAttribute("classes"));
    if(getBooleanAttribute("heatmap_output")) {
        setHeatmapOutput();
    } else {
        setSegmentationOutput();
    }
}

}