#include "BoundingBoxNetwork.hpp"
#include <FAST/Data/BoundingBox.hpp>

namespace fast {

void BoundingBoxNetwork::loadAttributes() {
	setThreshold(getFloatAttribute("threshold"));

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

	NeuralNetwork::loadAttributes();
}

void BoundingBoxNetwork::setAnchors(std::vector<std::vector<Vector2f>> anchors) {
    m_anchors = anchors;
}

BoundingBoxNetwork::BoundingBoxNetwork() {
    createInputPort<Image>(0);
    createOutputPort<BoundingBoxSet>(0);
    
    m_threshold = 0.5;

    createFloatAttribute("threshold", "Segmentation threshold", "Lower threshold of accepting a label", m_threshold);
    createStringAttribute("anchors", "Anchors", "Should be formatted like: x1,y1,x2,y2;x1,y1,x2,y2", "");
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
    return ordering == ImageOrdering::ChannelLast ? j + y*nrOfClasses + x*nrOfClasses*height : x + y*width + j*width*height;
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

void BoundingBoxNetwork::execute() {
    if(m_anchors.empty())
        throw Exception("No anchors was given to the bouding box network");

    run();

    mRuntimeManager->startRegularTimer("output_processing");
    const auto outputNodes = m_engine->getOutputNodes();

	int inputHeight;
    int inputWidth;
    auto inputShape = m_engine->getInputNodes().begin()->second.shape;
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelLast) {
        inputHeight = inputShape[1];
        inputWidth = inputShape[2];
    } else {
        inputHeight = inputShape[2];
        inputWidth = inputShape[3];
    }

    const auto ordering = ImageOrdering::ChannelLast;
    auto bbset = BoundingBoxSet::New();
    bbset->create();
    auto outputAccess = bbset->getAccess(ACCESS_READ_WRITE);
    int nodeIdx = 0;
    for(auto node : m_processedOutputData) {
		auto tensor = std::dynamic_pointer_cast<Tensor>(node.second);
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
        // Output tensor is 8x8x18, or 8x8x Anchors x (classes + 5)
		const int classes = channels / m_anchors[nodeIdx].size() - 5;

        // Loop over grid
        for(int y = 0; y < outputHeight; ++y) {
            for(int x = 0; x < outputWidth; ++x) {
				for(int a = 0; a < m_anchors[nodeIdx].size(); ++a) {
					float bestScore = 0.0f;
					uchar bestClass = 0;
					// Find best class
					float objectness = sigmoid(tensorData[getPosition(x, y, channels, a * 6 + 4, outputWidth, outputHeight, ordering)]);
					for(int classIdx = 0; classIdx < classes; ++classIdx) {
						float classPrediction = tensorData[getPosition(x, y, channels, a * 6 + 5 + classIdx, outputWidth, outputHeight, ordering)];
						float score = objectness * sigmoid(classPrediction);
						if(score >= m_threshold && score > bestScore) {
							bestClass = classIdx;
							bestScore = score;
						}
					}

					float t_x = tensorData[getPosition(x, y, channels, a * 6 + 0, outputWidth, outputHeight, ordering)];
					float t_y = tensorData[getPosition(x, y, channels, a * 6 + 1, outputWidth, outputHeight, ordering)];
					float t_w = tensorData[getPosition(x, y, channels, a * 6 + 2, outputWidth, outputHeight, ordering)];
					float t_h = tensorData[getPosition(x, y, channels, a * 6 + 3, outputWidth, outputHeight, ordering)];
					float b_w = m_anchors[nodeIdx][a].x() * std::exp(t_w);
					float b_h = m_anchors[nodeIdx][a].y() * std::exp(t_h);
					// Position is center
					float b_x = ((sigmoid(t_x) + x) / outputWidth) * inputWidth - 0.5f * b_w;
					float b_y = ((sigmoid(t_y) + y) / outputHeight) * inputHeight - 0.5f * b_h;
                    // Check if the bbox is properly inside the patch (at least 0.5 should be inside patch)
					//float intersectionArea = (std::min(b_x + b_w, (float)inputWidth) - std::max(b_x, 0.0f)) * (std::min(b_y + b_h, (float)inputHeight) - std::max(b_y, 0.0f));
                    if(bestScore >= m_threshold/* && intersectionArea/(b_w*b_h) >= 0.5*/) {
                        outputAccess->addBoundingBox(Vector2f(b_x, b_y), Vector2f(b_w, b_h), bestClass, bestScore);
                    }
				}
            }
        }
        ++nodeIdx;
    }
	addOutputData(0, bbset);
    mRuntimeManager->stopRegularTimer("output_processing");
}


void BoundingBoxNetwork::setThreshold(float threshold) {
    m_threshold = threshold;
}

}