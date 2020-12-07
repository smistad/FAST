#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include "TensorToSegmentation.hpp"
#include "InferenceEngine.hpp"

namespace fast {

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

TensorToSegmentation::TensorToSegmentation() {
    createInputPort<Tensor>(0);
    createOutputPort<Image>(0);

    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", m_threshold);
}

void TensorToSegmentation::execute() {
    auto tensor = getInputData<Tensor>();
    auto output = Image::New();

    auto shape = tensor->getShape();
    const int dims = shape.getDimensions();
    int outputHeight = shape[dims-3];
    int outputWidth = shape[dims-2];
    int outputDepth = 1;
    auto access = tensor->getAccess(ACCESS_READ);
    float* tensorData = access->getRawData();
    if(dims == 5) {
        outputDepth = shape[dims - 4];
    }
    const int size = outputWidth*outputHeight*outputDepth;

    auto data = make_uninitialized_unique<uchar[]>(size);
    const int nrOfClasses = tensor->getShape()[tensor->getShape().getDimensions()-1];
    const auto ordering = ImageOrdering::ChannelLast;
    for(int x = 0; x < size; ++x) {
        uchar maxClass = 0;
        for(uchar j = 1; j < nrOfClasses; j++) {
            if(tensorData[getPosition(x, nrOfClasses, j, size, ordering)] > m_threshold &&
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
    output->setSpacing(tensor->getSpacing());

    addOutputData(0, output);
}

void TensorToSegmentation::setThreshold(float threshold) {
    m_threshold = threshold;
}

float TensorToSegmentation::getThreshold() const {
    return m_threshold;
}

void TensorToSegmentation::loadAttributes() {
    setThreshold(getFloatAttribute("threshold"));
}

}