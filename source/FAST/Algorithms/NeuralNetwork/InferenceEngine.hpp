#pragma once

#include "FAST/Data/DataTypes.hpp"
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/TensorShape.hpp>

namespace fast {

/**
 * Different engines prefer different image dimension orderings.
 * C = channel, W = width, H = height
 */
enum class ImageOrdering {
    CHW,
    HWC
};

enum class NodeType {
    IMAGE,
    TENSOR,
};


/**
 * Abstract class for neural network inference engines (TensorFlow, TensorRT ++)
 */
class FAST_EXPORT InferenceEngine : public Object {
    public:
        typedef SharedPointer<InferenceEngine> pointer;
        struct NetworkNode {
            uint portID;
            NodeType type;
            TensorShape shape;
            SharedPointer<Tensor> data;
        };
        virtual void setFilename(std::string filename);
        virtual std::string getFilename() const;
        virtual void run() = 0;
        virtual void addInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
        virtual void addOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
        virtual NetworkNode getInputNode(std::string name) const;
        virtual NetworkNode getOutputNode(std::string name) const;
        virtual std::unordered_map<std::string, NetworkNode> getOutputNodes() const;
        virtual std::unordered_map<std::string, NetworkNode> getInputNodes() const;
        virtual void setInputData(std::string inputNodeName, SharedPointer<Tensor> tensor);
        virtual SharedPointer<Tensor> getOutputData(std::string inputNodeName);
        virtual void load() = 0;
        virtual bool isLoaded();
        virtual ImageOrdering getPreferredImageOrdering() const = 0;
        virtual std::string getName() const = 0;
        virtual std::string getDefaultFileExtension() const = 0;
    protected:
        virtual void setIsLoaded(bool loaded);

        std::unordered_map<std::string, NetworkNode> mInputNodes;
        std::unordered_map<std::string, NetworkNode> mOutputNodes;
    private:
        std::string m_filename = "";
        bool m_isLoaded = false;
};

}