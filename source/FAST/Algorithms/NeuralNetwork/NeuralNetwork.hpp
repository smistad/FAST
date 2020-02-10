#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include "InferenceEngine.hpp"

namespace fast {

class Image;
class Tensor;

/**
 * A class containing a list of data objects for inference, either FAST images or tensors
 */
class FAST_EXPORT InferenceDataList {
    public:
        explicit InferenceDataList(std::vector<SharedPointer<Image>> images) {
            m_images = images;
        }
        explicit InferenceDataList(std::vector<SharedPointer<Tensor>> tensors) {
            m_tensors = tensors;
        }
        InferenceDataList() {

        }
        std::vector<SharedPointer<Image>> getImages() const {
            if(!isImages())
                throw Exception("The inference that list contains tensors, not images");

            return m_images;
        }
        std::vector<SharedPointer<Tensor>> getTensors() const {
            if(!isTensors())
                throw Exception("The inference that list contains images, not tensors");

            return m_tensors;
        }
        bool isTensors() const { return !m_tensors.empty(); };
        bool isImages() const { return !m_images.empty(); };
        int getSize() const {
            return isImages() ? m_images.size() : m_tensors.size();
        }
    private:
        std::vector<SharedPointer<Image>> m_images;
        std::vector<SharedPointer<Tensor>> m_tensors;
};

class Sequence : public SimpleDataObject<InferenceDataList> {
	FAST_OBJECT(Sequence)
    public:
        void create(std::vector<SharedPointer<Image>> images) {
            mData = InferenceDataList(images);
        };
        void create(std::vector<SharedPointer<Tensor>> tensors) {
            mData = InferenceDataList(tensors);
        };
        typedef DataAccess<InferenceDataList>::pointer access;
    private:
        Sequence() {};
};

class Batch : public SimpleDataObject<InferenceDataList> {
	FAST_OBJECT(Batch)
    public:
        void create(std::vector<SharedPointer<Image>> images) {
            mData = InferenceDataList(images);
        };
        void create(std::vector<SharedPointer<Tensor>> tensors) {
            mData = InferenceDataList(tensors);
        };
        typedef DataAccess<InferenceDataList>::pointer access;
    private:
        Batch() {};
};

class FAST_EXPORT NeuralNetwork : public ProcessObject {
    FAST_OBJECT(NeuralNetwork)
    public:
        /**
         * Load a given netowrk file. This takes time.
         * @param filename
         */
        void load(std::string filename);
        /**
         * Load a network from memory provided as byte two byte vectors: model and weights
         * @param model
         * @param weights
         */
        void load(std::vector<uint8_t> model, std::vector<uint8_t> weights);
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
        void setInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
        void setOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
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

        virtual void run();

        SharedPointer<InferenceEngine> m_engine;

        std::unordered_map<std::string, std::vector<SharedPointer<Image>>> mInputImages;

        std::unordered_map<std::string, Tensor::pointer> processInputData();
        std::vector<SharedPointer<Image>> resizeImages(const std::vector<SharedPointer<Image>>& images, int width, int height, int depth);
        Tensor::pointer convertImagesToTensor(std::vector<SharedPointer<Image>> image, const TensorShape& shape, bool temporal);

    private:
        void execute();
};

}
