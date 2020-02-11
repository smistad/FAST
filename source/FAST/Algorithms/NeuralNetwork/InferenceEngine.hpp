#pragma once

#include "FAST/Data/DataTypes.hpp"
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/TensorShape.hpp>

// This is a macro for creating a load function for a given inference engine
// Need C linkage here (extern "C" to avoid mangled names of the load function on windows, see https://stackoverflow.com/questions/19422550/why-getprocaddress-is-not-working
// Export statement is needed for Windows
#define DEFINE_INFERENCE_ENGINE(classType, exportStatement)                                   \
extern "C" exportStatement                                                          \
InferenceEngine* load() {                                                                  \
      return new classType();                                                       \
}                                                                                      \

namespace fast {

/**
 * Different engines prefer different image dimension orderings.
 */
enum class ImageOrdering {
    ChannelFirst,
    ChannelLast
};

enum class NodeType {
    IMAGE,
    TENSOR,
};

enum class InferenceDeviceType {
    ANY,
    CPU,
    GPU,
    VPU,
    OTHER,
};

struct InferenceDeviceInfo {
    std::string name;
    InferenceDeviceType type;
    int index;
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
        virtual void setModelAndWeights(std::vector<uint8_t> model, std::vector<uint8_t> weights);
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
        /**
         * Set which device type the inference engine should use
         * (assuming the IE supports multiple devices like OpenVINO)
         * @param type
         */
        virtual void setDeviceType(InferenceDeviceType type);
        /**
         * Specify which device index and/or device type to use
         * @param index Index of the device to use. -1 means any device can be used
         * @param type
         */
        virtual void setDevice(int index = -1, InferenceDeviceType type = InferenceDeviceType::ANY);
        /**
         * Get a list of devices available for this inference engine.
         *
         * @return vector with info on each device
         */
        virtual std::vector<InferenceDeviceInfo> getDeviceList();

        virtual int getMaxBatchSize();
        virtual void setMaxBatchSize(int size);
    protected:
        virtual void setIsLoaded(bool loaded);

        std::unordered_map<std::string, NetworkNode> mInputNodes;
        std::unordered_map<std::string, NetworkNode> mOutputNodes;

        int m_deviceIndex = -1;
        InferenceDeviceType m_deviceType = InferenceDeviceType::ANY;
        int m_maxBatchSize = 1;

        std::vector<uint8_t> m_model;
        std::vector<uint8_t> m_weights;
    private:
        std::string m_filename = "";
        bool m_isLoaded = false;
};

}