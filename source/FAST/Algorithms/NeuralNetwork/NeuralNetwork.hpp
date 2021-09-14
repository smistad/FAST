#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include "InferenceEngine.hpp"

namespace fast {

class Image;
class Tensor;

/**
 * @brief A class containing a list of data objects for inference, either Image or Tensor objects.
 *
 * This is used as a container for Batch and Sequence data objects.
 */
class FAST_EXPORT InferenceDataList {
    public:
        explicit InferenceDataList(std::vector<std::shared_ptr<Image>> images) {
            m_images = images;
        }
        explicit InferenceDataList(std::vector<std::shared_ptr<Tensor>> tensors) {
            m_tensors = tensors;
        }
        InferenceDataList() {

        }
        std::vector<std::shared_ptr<Image>> getImages() const {
            if(!isImages())
                throw Exception("The inference data list contains tensors, not images");

            return m_images;
        }
        std::vector<std::shared_ptr<Tensor>> getTensors() const {
            if(!isTensors())
                throw Exception("The inference data list contains images, not tensors");

            return m_tensors;
        }
        bool isTensors() const { return !m_tensors.empty(); };
        bool isImages() const { return !m_images.empty(); };
        int getSize() const {
            return isImages() ? m_images.size() : m_tensors.size();
        }
    private:
        std::vector<std::shared_ptr<Image>> m_images;
        std::vector<std::shared_ptr<Tensor>> m_tensors;
};

/**
 * @brief Sequence data object
 *
 * A data object used for neural network processing of a sequence of data.
 * The sequence can consist of Image or Tensor objects.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT Sequence : public SimpleDataObject<InferenceDataList> {
	FAST_OBJECT_V4(Sequence)
    public:
        FAST_CONSTRUCTOR(Sequence, std::vector<std::shared_ptr<Image>>, images,)
        FAST_CONSTRUCTOR(Sequence, std::vector<std::shared_ptr<Tensor>>, tensors,)
};

/**
 * @brief Batch data object
 *
 * A data object used for batch processing in neural networks.
 * The batch consists of Image or Tensor objects.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT Batch : public SimpleDataObject<InferenceDataList> {
	FAST_OBJECT_V4(Batch)
    public:
        FAST_CONSTRUCTOR(Batch, std::vector<std::shared_ptr<Image>>, images,)
        FAST_CONSTRUCTOR(Batch, std::vector<std::shared_ptr<Tensor>>, tensors,)
};

class FAST_EXPORT NeuralNetworkNode {
    public:
        NeuralNetworkNode(std::string name, NodeType type = NodeType::UNSPECIFIED, TensorShape shape = TensorShape()) : name(name), type(type), shape(shape) {

        }
        std::string name;
        NodeType type;
        TensorShape shape;
};

/**
 * @defgroup neural-network Neural Network
 * Objects and functions related to Neural Network processing.
 */
/**
 * @brief Neural network process object
 *
 * This is the neural network process object base. All other neural network process objects should extend this class.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT NeuralNetwork : public ProcessObject {
    FAST_PROCESS_OBJECT(NeuralNetwork)
    public:
        // Python friendly constructor with almost all parameters
        FAST_CONSTRUCTOR(NeuralNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         float, meanIntensity, = 0.0f,
                         float, stanardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        )
#ifndef SWIG
        // C++ friendly create with parameters that must be set before loading
        FAST_CONSTRUCTOR(NeuralNetwork,
                 std::string, modelFilename,,
                 std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                 std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                 std::string, inferenceEngine, = "",
                 std::vector<std::string>, customPlugins, = std::vector<std::string>()
        )
#endif
        /**
         * Load a given network model file. This takes time. The second argument can be used
         * to specify files for loading custom plugins/operators needed by the network model.
         *
         * @param filename Path to network model file.
         * @param customPlugins Paths to custom plugins/operators which can be libraries (.so/.dll) or in the case of GPU/VPU OpenVINO: .xml files.
         */
        void load(std::string filename, std::vector<std::string> customPlugins = std::vector<std::string>());
        /**
         * Load a network from memory provided as byte two byte vectors: model and weights
         * The second argument can be used to specify files for loading custom plugins/operators
         * needed by the network model.
         *
         * @param model
         * @param weights
         * @param customPlugins paths to custom plugins/operators which can be libraries (.so/.dll) or in the case of GPU/VPU OpenVINO: .xml files.
         */
        void load(std::vector<uint8_t> model, std::vector<uint8_t> weights, std::vector<std::string> customPlugins = std::vector<std::string>());
        /**
         * Specify which inference engine to use
         * @param engine
         */
        void setInferenceEngine(InferenceEngine::pointer engine);
        /**
         * Specify which inference engine to use
         * @param engine
         */
        void setInferenceEngine(std::string engine);
        /**
         * Retrieve current inference engine
         * @return
         */
        InferenceEngine::pointer getInferenceEngine() const;
        void setInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = TensorShape());
        void setOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = TensorShape());
        /**
         * For each input value i: new_i = i*scale
         * @param scale
         */
        void setScaleFactor(float scale);
        /**
         * For each input value i: new_i = (i - mean)/std, this is applied after the scale factor
         * @param mean
         * @param std
         */
        void setMeanAndStandardDeviation(float mean, float std);
        /**
         * Intensities of input image will be clipped at these values
         * @param min
         * @param max
         */
        void setMinAndMaxIntensity(float min, float max);
        void setSignedInputNormalization(bool signedInputNormalization);
        void setPreserveAspectRatio(bool preserve);
        /**
         * Setting this parameter to true will flip the input image horizontally.
         * For pixel classification the output image will be flipped back.
         * @param flip
         */
        void setHorizontalFlipping(bool flip);

        /**
         * Set the temporal window for dynamic mode.
         * If window > 1, assume the second dimension of the input tensor is the number of timesteps.
         * If the window is set to 4, the frames t-3, t-2, t-1 and t, where t is the current timestep,
         * will be given as input to the network.
         *
         * @param window
         */
        void setTemporalWindow(uint window);

        virtual void setInputSize(std::string name, std::vector<int> size);

        void loadAttributes();

        virtual ~NeuralNetwork();
    protected:
        NeuralNetwork();
        bool mPreserveAspectRatio;
        bool mHorizontalImageFlipping = false;
        bool mSignedInputNormalization = false;
        int mTemporalWindow = 0;
        int m_batchSize;
        float mScaleFactor, mMean, mStd, mMinIntensity, mMaxIntensity;
        bool mMinAndMaxIntensitySet = false;
        Vector3f mNewInputSpacing;
        std::unordered_map<std::string, std::vector<int>> mInputSizes;
        std::unordered_map<int, DataObject::pointer> m_processedOutputData;

        virtual void run();

        std::shared_ptr<InferenceEngine> m_engine;

        std::unordered_map<std::string, std::vector<std::shared_ptr<Image>>> mInputImages;
        std::unordered_map<std::string, std::vector<std::shared_ptr<Tensor>>> mInputTensors;

        std::unordered_map<std::string, Tensor::pointer> processInputData();
        std::vector<std::shared_ptr<Image>> resizeImages(const std::vector<std::shared_ptr<Image>>& images, int width, int height, int depth);
        Tensor::pointer convertImagesToTensor(std::vector<std::shared_ptr<Image>> image, const TensorShape& shape, bool temporal);

        /**
         * Converts a tensor to channel last image ordering and takes care of frame data and spacing
         * @param tensor
         * @return
         */
        Tensor::pointer standardizeOutputTensorData(Tensor::pointer tensor, int sample = 0);

    private:
        void execute();
};

}
