#include <FAST/Data/Tensor.hpp>
#include "TensorToBoundingBoxSet.hpp"
#include <FAST/Data/BoundingBox.hpp>

namespace fast {

void TensorToBoundingBoxSet::loadAttributes() {
    setThreshold(getFloatAttribute("threshold"));

    // Duplicate of BoundingBoxNetwork
    std::vector<std::vector<Vector2f>> anchors;
    const auto level = split(getStringAttribute("anchors"), ";");
    for(auto&& part : level) {
        const auto parts = split(part, ",");
        std::vector<Vector2f> levelAnchors;
        for(int i = 0; i < parts.size(); i += 2) {
            levelAnchors.push_back(Vector2f(std::stof(parts[i]), std::stof(parts[i+1])));
        }
        anchors.push_back(levelAnchors);
    }
    setAnchors(anchors);
}

TensorToBoundingBoxSet::TensorToBoundingBoxSet(BoundingBoxNetworkType type, float threshold,
                                               std::vector<std::vector<Vector2f>> anchors) {
    createOutputPort(0, "BoundingBoxSet");

    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", 0.5f);
    createStringAttribute("anchors", "Anchors", "Should be formatted like: x1,y1,x2,y2;x1,y1,x2,y2", "");
    setType(type);
    setThreshold(threshold);
    setAnchors(anchors);
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
inline int getPosition(int x, int y, int nrOfClasses, int j, int width, int height) {
    return j + x*nrOfClasses + y*nrOfClasses*width;
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}
void TensorToBoundingBoxSet::execute() {

    std::map<int, Tensor::pointer> outputNodes; // Neural network output nodes
    for(int i = 0; i < getNrOfInputConnections(); ++i) {
        outputNodes[i] = getInputData<Tensor>(i);
    }

    //auto inputShape = outputNodes.begin()->second->getShape(); // TODO this should be size of input to network
    int inputHeight = 256;//inputShape[0]; // TODO fix
    int inputWidth = 256;//inputShape[1]; // TODO fix

    auto bbset = BoundingBoxSet::create();
    auto outputAccess = bbset->getAccess(ACCESS_READ_WRITE);
    int nodeIdx = 0;
    for(auto node : outputNodes) {
        auto tensor = std::dynamic_pointer_cast<Tensor>(node.second);
        Vector3f spacing = tensor->getSpacing();
        if(!tensor)
            throw Exception("Output data " + std::to_string(node.first) + " was not a tensor");
        const auto shape = tensor->getShape();
        auto access = tensor->getAccess(ACCESS_READ);
        const int dims = shape.getDimensions();
        if(dims != 3)
            throw Exception("Expected nr of output dimensions to be 3");

        int outputHeight;
        int outputWidth;
        int channels;
        outputHeight = shape[0];
        outputWidth = shape[1];
        channels = shape[2];
        float* tensorData = access->getRawData();
        // Output tensor is 8x8x18, or 8x8x (Anchors * (classes + 5))
        const int classes = channels / (int)m_anchors[nodeIdx].size() - 5;

        // Loop over grid
        for(int y = 0; y < outputHeight; ++y) {
            for(int x = 0; x < outputWidth; ++x) {
                for(int a = 0; a < m_anchors[nodeIdx].size(); ++a) {
                    float bestScore = 0.0f;
                    uchar bestClass = 0;
                    // Find best class
                    float objectness = sigmoid(tensorData[getPosition(x, y, channels, a * 6 + 4, outputWidth, outputHeight)]);
                    for(int classIdx = 0; classIdx < classes; ++classIdx) {
                        float classPrediction = tensorData[getPosition(x, y, channels, a * 6 + 5 + classIdx, outputWidth, outputHeight)];
                        float score = objectness * sigmoid(classPrediction);
                        if(score >= m_threshold && score > bestScore) {
                            bestClass = classIdx;
                            bestScore = score;
                        }
                    }

                    float t_x = tensorData[getPosition(x, y, channels, a * 6 + 0, outputWidth, outputHeight)];
                    float t_y = tensorData[getPosition(x, y, channels, a * 6 + 1, outputWidth, outputHeight)];
                    float t_w = tensorData[getPosition(x, y, channels, a * 6 + 2, outputWidth, outputHeight)];
                    float t_h = tensorData[getPosition(x, y, channels, a * 6 + 3, outputWidth, outputHeight)];
                    float b_w = m_anchors[nodeIdx][a].x() * std::exp(t_w);
                    float b_h = m_anchors[nodeIdx][a].y() * std::exp(t_h);
                    // Position is center
                    float b_x = ((sigmoid(t_x) + x) / outputWidth) * inputWidth - 0.5f * b_w;
                    float b_y = ((sigmoid(t_y) + y) / outputHeight) * inputHeight - 0.5f * b_h;
                    // Check if the bbox is properly inside the patch (at least 0.5 should be inside patch)
                    //float intersectionArea = (std::min(b_x + b_w, (float)inputWidth) - std::max(b_x, 0.0f)) * (std::min(b_y + b_h, (float)inputHeight) - std::max(b_y, 0.0f));
                    if (bestScore >= m_threshold/* && intersectionArea/(b_w*b_h) >= 0.5*/) {
                        outputAccess->addBoundingBox(Vector2f(b_x*spacing.x(), b_y*spacing.y()), Vector2f(b_w*spacing.x(), b_h*spacing.y()), bestClass + 1, bestScore);
                    }
                }
            }
        }
        ++nodeIdx;
    }
    addOutputData(0, bbset);
}

void TensorToBoundingBoxSet::setType(BoundingBoxNetworkType type) {
    m_type = type;
}

void TensorToBoundingBoxSet::setInputConnection(DataChannel::pointer channel) {
    // This is hack to connect all output ports to this PO:
    // For each output port of parent: create an input port and connect it to parent.
    for(int i = 0; i < channel->getProcessObject()->getNrOfOutputPorts(); ++i) {
        createInputPort(i, "Tensor");
        ProcessObject::setInputConnection(i, channel->getProcessObject()->getOutputPort(i));
    }
}

void TensorToBoundingBoxSet::setInputConnection(uint portID, DataChannel::pointer channel) {
    if(getNrOfInputPorts() == 0) { // Only once
        setInputConnection(channel);
    } else {
        ProcessObject::setInputConnection(portID, channel);
    }
}

void TensorToBoundingBoxSet::setThreshold(float threshold) {
    m_threshold = threshold;
}

void TensorToBoundingBoxSet::setAnchors(std::vector<std::vector<Vector2f>> anchors) {
    m_anchors = anchors;
}

void TensorToBoundingBoxSet::setNrOfInputNodes(int nr) {
    for(int i = 0; i < nr; ++i)
        createInputPort(i, "Tensor");
}


}