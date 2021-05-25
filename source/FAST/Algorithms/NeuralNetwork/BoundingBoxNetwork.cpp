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



void BoundingBoxNetwork::execute() {
    if(m_anchors.empty())
        throw Exception("No anchors was given to the bouding box network");

    run();

    mRuntimeManager->startRegularTimer("output_processing");

    mRuntimeManager->stopRegularTimer("output_processing");
}


void BoundingBoxNetwork::setThreshold(float threshold) {
    m_threshold = threshold;
}

}