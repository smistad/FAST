#include "InferenceEngine.hpp"

namespace fast {

void InferenceEngine::setModelAndWeights(std::vector<uint8_t> model, std::vector<uint8_t> weights) {
    m_model = model;
    m_weights = weights;
}

void InferenceEngine::setFilename(std::string filename) {
    m_filename = filename;
}

std::string InferenceEngine::getFilename() const {
    return m_filename;
}

bool InferenceEngine::isLoaded() {
    return m_isLoaded;
}

void InferenceEngine::setIsLoaded(bool loaded) {
    m_isLoaded = loaded;
}

void InferenceEngine::addInputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
    NetworkNode node;
    node.portID = portID;
    node.type = type;
    node.shape = shape;
    mInputNodes[name] = node;
}

void InferenceEngine::addOutputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
    NetworkNode node;
    node.portID = portID;
    node.type = type;
    node.shape = shape;
    mOutputNodes[name] = node;
}

InferenceEngine::NetworkNode InferenceEngine::getInputNode(std::string name) const {
    return mInputNodes.at(name);
}

InferenceEngine::NetworkNode InferenceEngine::getOutputNode(std::string name) const {
    return mOutputNodes.at(name);
}

std::unordered_map<std::string, InferenceEngine::NetworkNode> InferenceEngine::getOutputNodes() const {
    return mOutputNodes;
}

std::unordered_map<std::string, InferenceEngine::NetworkNode> InferenceEngine::getInputNodes() const {
    return mInputNodes;
}


void InferenceEngine::setInputData(std::string nodeName, SharedPointer<Tensor> tensor) {
	mInputNodes.at(nodeName).data = tensor;
}

SharedPointer<fast::Tensor> InferenceEngine::getOutputData(std::string nodeName) {
    return mOutputNodes.at(nodeName).data;
}

void InferenceEngine::setDeviceType(InferenceDeviceType type) {
    m_deviceType = type;
}

void InferenceEngine::setDevice(int index, InferenceDeviceType type) {
    m_deviceIndex = index;
    m_deviceType = type;
}

std::vector<InferenceDeviceInfo> InferenceEngine::getDeviceList() {
    throw Exception("getDeviceList is not supported for the inference engine " + getName());
}

void InferenceEngine::setMaxBatchSize(int size) {
    m_maxBatchSize = size;
}

int InferenceEngine::getMaxBatchSize() {
    return m_maxBatchSize;
}

}