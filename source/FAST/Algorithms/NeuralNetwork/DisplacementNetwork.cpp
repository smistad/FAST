#include "DisplacementNetwork.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

DisplacementNetwork::DisplacementNetwork() {
    createInputPort<Sequence>(0);
    createOutputPort<Image>(0);
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

void DisplacementNetwork::execute() {
    // Prepare input node: Should be two images in a sequence
    run();

    Tensor::pointer tensor = m_engine->getOutputNodes().begin()->second.data;
    const auto shape = tensor->getShape();
    if(shape[0] != 1)
        throw Exception("Displacement network only support batch size 1 atm");
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
    auto image = Image::New();
    if(ordering == ImageOrdering::ChannelFirst) {
        // Convert to channel last
        const int nrOfClasses = tensor->getShape()[0];
        auto newTensorData = make_uninitialized_unique<float[]>(size*nrOfClasses);
        for(int x = 0; x < size; ++x) {
            for(uchar j = 0; j < nrOfClasses; ++j) {
                newTensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelLast)] = tensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelFirst)];
            }
        }
        auto oldShape = tensor->getShape();
        image->create(outputWidth, outputHeight, TYPE_FLOAT, 2, std::move(newTensorData));
    } else {
        image->create(outputWidth, outputHeight, TYPE_FLOAT, 2, std::move(tensorData));
    }
    image->setSpacing(mNewInputSpacing);
    SceneGraph::setParentNode(image, mInputImages.begin()->second[0]);
    addOutputData(0, image);
}

ImagesToSequence::ImagesToSequence() {
    createInputPort<Image>(0);
    createOutputPort<Sequence>(0);
}

void ImagesToSequence::execute() {
    auto newImage = getInputData<Image>(0);
    if(m_queue.size() > 0)
        m_queue.pop_front();
    while(m_queue.size() < m_sequenceSize)
        m_queue.push_back(newImage);

    auto sequence = Sequence::New();
    std::vector<Image::pointer> vector{m_queue.begin(), m_queue.end()};
    sequence->create(vector);
    addOutputData(0, sequence);
}

void ImagesToSequence::setSequenceSize(int size) {
    if(size <= 0)
        throw Exception("Sequence size must be larger than 0");
    m_sequenceSize = size;
}

}