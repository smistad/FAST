#include "BoundingBoxNetwork.hpp"
#include <FAST/Data/BoundingBox.hpp>

namespace fast {

void BoundingBoxNetwork::loadAttributes() {
	setThreshold(getFloatAttribute("threshold"));
	NeuralNetwork::loadAttributes();
}

BoundingBoxNetwork::BoundingBoxNetwork() {
    createInputPort<Image>(0);

    createOutputPort<BoundingBoxSet>(0);
    
    m_threshold = 0.5;

    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", m_threshold);
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

void BoundingBoxNetwork::execute() {

    run();

    mRuntimeManager->startRegularTimer("output_processing");
    Tensor::pointer tensor = m_engine->getOutputNodes().begin()->second.data;
    const auto shape = tensor->getShape();
    if(shape[0] != 1)
        throw Exception("BoundingBoxNetwork only support batch size 1 atm");

    auto access = tensor->getAccess(ACCESS_READ);
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
	//SceneGraph::setParentNode(tensor, mInputImages.begin()->second[0]);
	addOutputData(0, tensor);
    mRuntimeManager->stopRegularTimer("output_processing");
}


void BoundingBoxNetwork::setThreshold(float threshold) {
    m_threshold = threshold;
}

}