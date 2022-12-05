#pragma once

#include "FAST/Data/DataTypes.hpp"
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/TensorShape.hpp>
#include <map>

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
    UNSPECIFIED,
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
 * Neural network modell formats
 */
enum class ModelFormat {
    PROTOBUF,
    SAVEDMODEL,
    ONNX,
    OPENVINO,
    UFF
};

/**
 * Get model format file extension.
 */
FAST_EXPORT std::string getModelFileExtension(ModelFormat format);

/**
 * Get model format of the given file.
 */
FAST_EXPORT ModelFormat getModelFormat(std::string filename);

/**
 * Get name of model format as string
 * @param format
 * @return
 */
FAST_EXPORT std::string getModelFormatName(ModelFormat format);

/**
 * Abstract class for neural network inference engines (TensorFlow, TensorRT ++)
 */
class FAST_EXPORT InferenceEngine : public Object {
    public:
        typedef std::shared_ptr<InferenceEngine> pointer;
        struct NetworkNode {
            uint portID;
            NodeType type;
            TensorShape shape;
            std::shared_ptr<Tensor> data;
        };
        virtual void setFilename(std::string filename);
        virtual void setModelAndWeights(std::vector<uint8_t> model, std::vector<uint8_t> weights);
        virtual std::string getFilename() const;
        virtual void run() = 0;
        virtual void addInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = TensorShape());
        virtual void addOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = TensorShape());
        virtual void setInputNodeShape(std::string name, TensorShape shape);
        virtual void setOutputNodeShape(std::string name, TensorShape shape);
        virtual NetworkNode getInputNode(std::string name) const;
        virtual NetworkNode getOutputNode(std::string name) const;
        virtual std::map<std::string, NetworkNode> getOutputNodes() const;
        virtual std::map<std::string, NetworkNode> getInputNodes() const;
        virtual void setInputData(std::string inputNodeName, std::shared_ptr<Tensor> tensor);
        virtual std::shared_ptr<Tensor> getOutputData(std::string inputNodeName);
        virtual void load() = 0;
        virtual bool isLoaded() const;
        virtual ImageOrdering getPreferredImageOrdering() const = 0;
        virtual std::string getName() const = 0;
        virtual std::vector<ModelFormat> getSupportedModelFormats() const = 0;
        virtual ModelFormat getPreferredModelFormat() const = 0;
        virtual bool isModelFormatSupported(ModelFormat format);
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
        /**
         * Load a custom operator (op), plugin. Must be called before load()
         *
         * @param filename path to library (.so/.dll) or in the case of GPU/VPU OpenVINO .xml files.
         */
        virtual void loadCustomPlugins(std::vector<std::string> filenames);
        /**
         * @brief Set dimension image ordering manually.
         * E.g. channel last or channel-first.
         *
         * @param ordering
         */
        virtual void setImageOrdering(ImageOrdering ordering);
    protected:
        virtual void setIsLoaded(bool loaded);

        std::map<std::string, NetworkNode> mInputNodes;
        std::map<std::string, NetworkNode> mOutputNodes;

        int m_deviceIndex = -1;
        InferenceDeviceType m_deviceType = InferenceDeviceType::ANY;
        int m_maxBatchSize = 1;

        std::vector<uint8_t> m_model;
        std::vector<uint8_t> m_weights;
        ImageOrdering m_imageOrdering;
    private:
        std::string m_filename = "";
        bool m_isLoaded = false;
};

}