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
inline int getPosition(int x, int y, int nrOfClasses, int j, int width, int height, ImageOrdering ordering) {
    return ordering == ImageOrdering::ChannelLast ? (y + x*height)*nrOfClasses + j : (x + y*width) + j*(width*height);
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

void BoundingBoxNetwork::execute() {

    run();

    mRuntimeManager->startRegularTimer("output_processing");
    Tensor::pointer tensor = m_engine->getOutputNodes().begin()->second.data;
    //std::cout << "Nr of output nodes " << m_engine->getOutputNodes().size() << std::endl;
    const auto shape = tensor->getShape();
    if(shape[0] != 1)
        throw Exception("BoundingBoxNetwork only support batch size 1 atm");

    auto access = tensor->getAccess(ACCESS_READ);
    const int dims = shape.getDimensions();
    if(dims != 4)
        throw Exception("Expected nr of output dimensions to be 4");

    int channels;
	int outputHeight;
    int outputWidth;
	int inputHeight;
    int inputWidth;
    auto inputShape = m_engine->getInputNodes().begin()->second.shape;
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
        outputHeight = shape[2];
        outputWidth = shape[3];
        channels = shape[1];
        inputHeight = inputShape[2];
        inputWidth = inputShape[3];
    } else {
        outputHeight = shape[1];
        outputWidth = shape[2];
        channels = shape[3];
        inputHeight = inputShape[1];
        inputWidth = inputShape[2];
    }
    float* tensorData = access->getRawData();
    auto ordering = m_engine->getPreferredImageOrdering();

    const int size = outputWidth*outputHeight;
    // TODO reuse some of the output processing in NN
    tensor->deleteDimension(0); // TODO assuming batch size is 1, remove this dimension

    auto bbset = BoundingBoxSet::New();
    bbset->create();
    auto outputAccess = bbset->getAccess(ACCESS_READ_WRITE);
    // Output tensor is 8x8x18, or 8x8x Anchors x (classes + 5)
    //std::vector<Vector2f> anchors = {{10, 10}, {18, 17}, {19, 26}};
    std::vector<Vector2f> anchors = {{28, 21}, {30, 31}, {39, 43}};
	int classes = channels / anchors.size() - 5;
    // Loop over grid
	for(int y = 0; y < outputHeight; ++y) {
        for(int x = 0; x < outputWidth; ++x) {
            for(int a = 0; a < anchors.size(); ++a) {
                //std::cout << "For anchor a: " << a << std::endl;
                float t_x = tensorData[getPosition(x, y, channels, a * 6 + 0, outputWidth, outputHeight, ordering)];
                float t_y = tensorData[getPosition(x, y, channels, a * 6 + 1, outputWidth, outputHeight, ordering)];
                float t_w = tensorData[getPosition(x, y, channels, a * 6 + 2, outputWidth, outputHeight, ordering)];
                float t_h = tensorData[getPosition(x, y, channels, a * 6 + 3, outputWidth, outputHeight, ordering)];
                float objectness = tensorData[getPosition(x, y, channels, a * 6 + 4, outputWidth, outputHeight, ordering)];
                float classPrediction = tensorData[getPosition(x, y, channels, a * 6 + 5, outputWidth, outputHeight, ordering)];
                //std::cout << "Class prediction and objectness: " << classPrediction << " " << objectness << std::endl;
                //std::cout << "bbox offsets: " << t_x << " " << t_y << " " << t_w << " " << t_h << std::endl;
                float b_x = ((sigmoid(t_x) + x) / outputWidth) * inputWidth;
                float b_y = ((sigmoid(t_y) + y) / outputHeight) * inputHeight;
                float b_w = anchors[a].x() * std::exp(t_w);
                float b_h = anchors[a].y() * std::exp(t_h);
                float score = sigmoid(objectness) * sigmoid(classPrediction);
                //std::cout << "bbox: " << b_x << " " << b_y << " " << b_w << " " << b_h << std::endl;
                //std::cout << score << std::endl;
                if(score >= m_threshold)
					outputAccess->addBoundingBox(Vector2f(b_x, b_y), Vector2f(b_w, b_h));
            }
        }
	}
	addOutputData(0, bbset);
    mRuntimeManager->stopRegularTimer("output_processing");
}


void BoundingBoxNetwork::setThreshold(float threshold) {
    m_threshold = threshold;
}

}