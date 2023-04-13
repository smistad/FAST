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
 * @brief A struct/class representing a neural network input/output node
 * @ingroup neural-network
 */
class FAST_EXPORT NeuralNetworkNode {
public:
    /**
     * @brief Constructor
     *
     * @param name Name of node, must match name of node in network file
     * @param type
     * @param shape
     * @param id Input/Output port id
     * @param minShape If node has dynamic shape, you can set the minimum shape for optimizations.
     *    Note that this is required for the TensorRT engine.
     * @param maxShape If node has dynamic shape, you can set the maximum shape for optimizations.
     *    Note that this is required for the TensorRT engine.
     */
    NeuralNetworkNode(std::string name,
                      NodeType type = NodeType::UNSPECIFIED,
                      fast::TensorShape shape = fast::TensorShape(),
                      uint id = 0,
                      fast::TensorShape minShape = fast::TensorShape(),
                      fast::TensorShape maxShape = fast::TensorShape(),
                      fast::TensorShape optShape = fast::TensorShape()
    ) : name(name), type(type), shape(shape), id(id), minShape(minShape), maxShape(maxShape), optShape(optShape) {

    }

#ifndef SWIG
    NeuralNetworkNode() {name = "uninitialized";};
#endif
    uint id;
    std::string name;
    NodeType type;
    fast::TensorShape shape; // namespace is needed here for swig/pyfast to work for some reason
    fast::TensorShape optShape;
    fast::TensorShape minShape;
    fast::TensorShape maxShape;
    std::shared_ptr<fast::Tensor> data;
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
        virtual void setFilename(std::string filename);
        virtual void setModelAndWeights(std::vector<uint8_t> model, std::vector<uint8_t> weights);
        virtual std::string getFilename() const;
        virtual void run() = 0;
        virtual void addInputNode(NeuralNetworkNode node);
        virtual void addOutputNode(NeuralNetworkNode node);
        virtual void setInputNodeShape(std::string name, TensorShape shape);
        virtual void setOutputNodeShape(std::string name, TensorShape shape);
        virtual NeuralNetworkNode getInputNode(std::string name) const;
        virtual NeuralNetworkNode getOutputNode(std::string name) const;
        virtual std::map<std::string, NeuralNetworkNode> getOutputNodes() const;
        virtual std::map<std::string, NeuralNetworkNode> getInputNodes() const;
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

        std::map<std::string, NeuralNetworkNode> mInputNodes;
        std::map<std::string, NeuralNetworkNode> mOutputNodes;

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