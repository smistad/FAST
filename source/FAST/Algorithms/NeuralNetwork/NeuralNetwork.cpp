#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Tensor.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "InferenceEngineManager.hpp"


namespace fast {


void NeuralNetwork::setScaleFactor(float factor) {
    mScaleFactor = factor;
}

void NeuralNetwork::setPreserveAspectRatio(bool preserve) {
    mPreserveAspectRatio = preserve;
}

void NeuralNetwork::setHorizontalFlipping(bool flip) {
	mHorizontalImageFlipping = flip;
}

void NeuralNetwork::loadAttributes() {
    auto preferredEngine = getStringAttribute("inference-engine");
    if(!preferredEngine.empty())
        setInferenceEngine(preferredEngine);
    m_engine->setFilename(getStringAttribute("model"));
    
    // TODO input nodes, shapes, and tensor/images type
    auto outputNames = getStringListAttribute("output-names");
    int i = 0;
    for (auto&& name : outputNames) {
        setOutputNode(i, name);
    }
    setScaleFactor(getFloatAttribute("scale-factor"));
    setSignedInputNormalization(getBooleanAttribute("signed-input-normalization"));
    setPreserveAspectRatio(getBooleanAttribute("preserve-aspect"));
}

NeuralNetwork::NeuralNetwork() {
	mPreserveAspectRatio = false;
	mScaleFactor = 1.0f;
	mMean = 0.0;
	mStd = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
    
	createStringAttribute("model", "Model path", "Path to the neural network model", "");
    createStringAttribute("inference-engine", "Inference Engine", "Manually set the inference engine to be used to execute this neural network.", "");
	createFloatAttribute("scale-factor", "Scale factor", "Scale factor", mScaleFactor);
	createStringAttribute("output-names", "Output names", "Name of output nodes", "");
	createBooleanAttribute("signed-input-normalization", "Signed input normalization", "Normalize input to -1 and 1 instead of 0 to 1.", false);
    createBooleanAttribute("preserve-aspect", "Preserve aspect ratio of input images", "", mPreserveAspectRatio);

	m_engine = InferenceEngineManager::loadBestAvailableEngine();
	reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
}

std::unordered_map<std::string, Tensor::pointer> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, Tensor::pointer> tensors;
    m_batchSize = -1;
    for(auto inputNode : m_engine->getInputNodes()) {
        auto shape = inputNode.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unable to deduce input shape from network file. "
                            "Either export the file with shape information or supply the input shape manually using setInputNode.");

        SharedPointer<DataObject> data = getInputData<DataObject>(inputNode.second.portID);
        mRuntimeManager->startRegularTimer("input_processing");

        bool containsSequence = false;
        // Check if data object is an tensor by doing a dynamic cast
        Tensor::pointer tensor = std::dynamic_pointer_cast<Tensor>(data);
        if(!tensor) {
            if(shape.getDimensions() < 4)
                throw Exception("Trying to attach an image to an input node with shape with fewer than 4 dimensions");

            // If not a tensor, data is either a Batch of images or a single Image
            Batch::pointer batch = std::dynamic_pointer_cast<Batch>(data);
            std::vector<Image::pointer> inputImages;
            std::vector<Tensor::pointer> inputTensors;
            if(batch) {
                Batch::access access = batch->getAccess(ACCESS_READ);
                auto dataList = access->getData();
                if(dataList.isImages()) {
                    inputImages = dataList.getImages();
                } else {
                    inputTensors = dataList.getTensors();
                }
                
                if(m_batchSize == -1) {
                    m_batchSize = dataList.getSize();
                } else {
                    throw Exception("Inconsistent batch size accross input nodes");
                }
            } else {
                m_batchSize = 1;
                Sequence::pointer sequence = std::dynamic_pointer_cast<Sequence>(data);
                if(sequence) {
                    Sequence::access access = sequence->getAccess(ACCESS_READ);
                    auto dataList = access->getData();
                    if(dataList.isImages()) {
                        inputImages = dataList.getImages();
                    } else {
                        inputTensors = dataList.getTensors();
                    }
                
                    containsSequence = true;
                    if(shape[1] == -1) {
                        // Nr of timesteps unknown in the shape, set it
                        shape[1] = dataList.getSize();
                    } else {
                        // TODO this check is probably not necessary?
                        // Nr of timesteps is specified, check that it matches
                        if(shape[1] != inputImages.size()) {
                            throw Exception("The number of timesteps in the input node shape doesn't match the number of images in the sequence");
                        }
                    }
                } else {
                    // Single image
                    Image::pointer image = std::dynamic_pointer_cast<Image>(data);
                    if(mTemporalWindow > 0) {
                        containsSequence = true;
                        shape[1] = mTemporalWindow;
                        inputImages = mInputImages[inputNode.first];
                        inputImages.push_back(image);
                        while(inputImages.size() < mTemporalWindow)
                            inputImages.push_back(image);

                        // Remove extra
                        inputImages.erase(inputImages.begin(), inputImages.begin() + inputImages.size() - mTemporalWindow);
                        if(inputImages.size() != mTemporalWindow)
                            throw Exception("Error");
                    } else {
                        inputImages = {image};
                    }
                }
            }

            if(!inputImages.empty()) { // We have a list of images to preprocess
                mInputImages[inputNode.first] = inputImages;

                // Resize images to fit input
                const int dims = shape.getDimensions();
                int height = shape[dims - 3];
                int width = shape[dims - 2];
                if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
                    height = shape[dims - 2];
                    width = shape[dims - 1];
                }
                int depth = 1;
                int timesteps = 0;
                if(containsSequence) {
                    // Temporal input
                    timesteps = shape[1];
                    if(dims == 6) // 3D
                        depth = m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelLast ? shape[dims - 4] : shape[
                                dims - 3];
                } else {
                    if(dims == 5) // 3D
                        depth = m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelLast ? shape[dims - 4] : shape[
                                dims - 3];
                }
                auto inputImages2 = resizeImages(inputImages, width, height, depth);

                // Convert images to tensors
                shape[0] = m_batchSize;
                tensors[inputNode.first] = convertImagesToTensor(inputImages2, shape, containsSequence);
            } else {
                // TODO fix ordering if necessary
                // We have a list of tensors, convert the list of tensors into a single tensor
                auto shape = inputTensors.front()->getShape();
                m_batchSize = shape[0];
                shape.insertDimension(0, inputTensors.size());
                auto tensor = Tensor::New();
                tensor->create(shape);
                auto access = tensor->getAccess(ACCESS_READ_WRITE);
                float* data = access->getRawData();
                for(int i = 0; i < inputTensors.size(); ++i) {
                    auto accessRead = inputTensors[i]->getAccess(ACCESS_READ);
                    const int totalSize = accessRead->getShape().getTotalSize();
                    std::memcpy(&data[i*totalSize], accessRead->getRawData(), totalSize*sizeof(float));
                }
            }
        } else {
            // TODO fix ordering if necessary
            // Input is a tensor, no conversion needed
            tensors[inputNode.first] = tensor;
        }
        mRuntimeManager->stopRegularTimer("input_processing");
	}

	return tensors;
}

void NeuralNetwork::run() {
    // TODO move load and input processing to execute? or a separate function?
    // Check if network is loaded, if not do it
    if(!m_engine->isLoaded())
        m_engine->load();

    // Prepare input data
	auto inputTensors = processInputData();
	// Give input tensors to inference engine
    for(const auto &node : m_engine->getInputNodes()) {
        m_engine->setInputData(node.first, inputTensors[node.first]);
    }

	// Run network
    mRuntimeManager->startRegularTimer("inference");
	m_engine->run();
    mRuntimeManager->stopRegularTimer("inference");
}

void NeuralNetwork::execute() {
    // Load, prepare input and run network
    run();

    mRuntimeManager->startRegularTimer("output_processing");
	// Collect output data of network and add to output ports
    for(const auto &node : m_engine->getOutputNodes()) {
        // TODO if input was a batch, the output should be converted to a batch as well
        // TODO and any frame data (such as patch info should be transferred)
        auto tensor = m_engine->getOutputData(node.first);

        if(m_batchSize > 1) {
            // Create a batch of tensors
            std::vector<Tensor::pointer> tensorList;
            auto tensorAccess = tensor->getAccess(ACCESS_READ);
            const float* rawTensorData = tensorAccess->getRawData();
            // Calculate sample size
            auto shape = tensor->getShape();
            int size = 1;
            auto newShape = TensorShape();
            for(int i = 1; i < shape.getDimensions(); ++i) {
                size *= shape[i];
                newShape.addDimension(shape[i]);
            }

            for(int i = 0; i < m_batchSize; ++i) {
                auto newTensor = Tensor::New();
                auto newData = make_uninitialized_unique<float[]>(size);
                std::memcpy(newData.get(), &(rawTensorData[i*size]), size*sizeof(float));
                newTensor->create(std::move(newData), newShape);
                tensorList.push_back(newTensor);
                for(auto& inputNode : m_engine->getInputNodes()) {
                    // TODO assuming input are images here:
                    for(auto &&frameData : mInputImages[inputNode.first][i]->getFrameData()) {
                        newTensor->setFrameData(frameData.first, frameData.second);
                    }
                    for(auto &&lastFrame : mInputImages[inputNode.first][i]->getLastFrame())
                        newTensor->setLastFrame(lastFrame);
                }
            }
            auto outputBatch = Batch::New();
            outputBatch->create(tensorList);
            addOutputData(node.second.portID, outputBatch);
        } else {
            // Remove first dimension as it is 1, due to batch size 1
            tensor->deleteDimension(0);
            for(auto& inputNode : m_engine->getInputNodes()) {
                // TODO assuming input are images here: Should also be able to handle tensors
                for(auto &&frameData : mInputImages[inputNode.first][0]->getFrameData()) {
                    tensor->setFrameData(frameData.first, frameData.second);
                }
                for(auto &&lastFrame : mInputImages[inputNode.first][0]->getLastFrame())
                    tensor->setLastFrame(lastFrame);
            }
            addOutputData(node.second.portID, tensor);
        }
    }
    mRuntimeManager->stopRegularTimer("output_processing");
}

Tensor::pointer NeuralNetwork::convertImagesToTensor(std::vector<Image::pointer> images, const TensorShape& shape, bool temporal) {
    if(shape.getUnknownDimensions() > 0)
        throw Exception("Shape must be known at this time");

    // Create input tensor
    auto values = make_uninitialized_unique<float[]>(shape.getTotalSize());

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);
    int depth = 1;
    int timesteps = 0;
    std::string kernelName;
    const int dims = shape.getDimensions();
    int channels = shape[dims-1];
    int width = shape[dims-2];
    int height = shape[dims-3];
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
        channels = shape[dims-3];
        width = shape[dims-1];
        height = shape[dims-2];
    }
    if(temporal) {
        timesteps = shape[1];
        if(shape[0] != 1)
            throw Exception("Batch of sequences for NN processing not supported yet!");
    }
    if(images[0]->getDimensions() == 2) {
        kernelName = "normalize2DInput";
        if((!temporal && shape.getDimensions() != 4) || (temporal && shape.getDimensions() != 5))
            throw Exception("Incorrect shape size");
        depth = 1;
    } else {
        kernelName = "normalize3DInput";
        if((!temporal && shape.getDimensions() != 5) || (temporal && shape.getDimensions() != 6))
            throw Exception("Incorrect shape size");
        if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
            channels = shape[dims-4];
            depth = shape[dims-3];
            height = shape[dims-2];
            width = shape[dims-1];
        } else {
            depth = shape[dims - 4];
        }
    }
    cl::Kernel kernel(program, kernelName.c_str());
    const std::size_t size = width*height*depth*channels; // nr of elements per image
    for(int i = 0; i < images.size(); ++i) {
        auto image = images[i];
        if(image->getWidth() != width ||
            image->getHeight() != height ||
            image->getDepth() != depth)
            throw Exception("Input image sent to executeNetwork was of incorrect size: " +
                    std::to_string(image->getWidth()) + "," + std::to_string(image->getHeight()) + "," +
                    std::to_string(image->getDepth()) + ". Expected: " + std::to_string(width) + ", " +
                    std::to_string(height) + "," + std::to_string(depth) + ".");
        if(image->getNrOfChannels() != channels)
            throw Exception("Input image sent to executeNetwork has incorrect nr of channels: " +
                    std::to_string(image->getNrOfChannels())+ ". Expected: " + std::to_string(channels) + ".");
        OpenCLImageAccess::pointer access = image->getOpenCLImageAccess(ACCESS_READ, device);
        cl::Buffer buffer(
                device->getContext(),
                CL_MEM_WRITE_ONLY,
                sizeof(float) * size
        );
        kernel.setArg(1, buffer);
        kernel.setArg(2, mScaleFactor);
        kernel.setArg(3, mMean);
        kernel.setArg(4, mStd);
        kernel.setArg(5, (int) (mSignedInputNormalization ? 1 : 0));
        kernel.setArg(6, (int) (mHorizontalImageFlipping ? 1 : 0));
        kernel.setArg(7, image->getNrOfChannels());
        kernel.setArg(8, mMinIntensity);
        kernel.setArg(9, mMaxIntensity);
        kernel.setArg(10, (int)(mMinAndMaxIntensitySet ? 1 : 0));
        kernel.setArg(11, (int)(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst ? 1 : 0));
        cl::NDRange globalSize;
        if(image->getDimensions() == 2) {
            kernel.setArg(0, *access->get2DImage());
            globalSize = cl::NDRange(width, height);
        } else {
            kernel.setArg(0, *access->get3DImage());
            globalSize = cl::NDRange(width, height, depth);
        }

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );

        // Read data directly into slice
        device->getCommandQueue().enqueueReadBuffer(buffer, CL_TRUE, 0, sizeof(float) * size,
                                                    values.get() + i*size);
    }

    auto tensor = Tensor::New();
    tensor->create(std::move(values), shape);
    return tensor;
}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images, int width, int height, int depth) {
    mRuntimeManager->startRegularTimer("image input resize");
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(width != image->getWidth() || height != image->getHeight() || depth != image->getDepth()) {
			// Only resize if needed
            auto resizer = ImageResizer::New();
            resizer->setWidth(width);
            resizer->setHeight(height);
            resizer->setDepth(depth);
            resizer->setInputData(image);
			resizer->setPreserveAspectRatio(mPreserveAspectRatio);
			DataChannel::pointer port = resizer->getOutputPort();
            resizer->update();
            Image::pointer resizedImage = port->getNextFrame<Image>();
            mNewInputSpacing = resizedImage->getSpacing();
            resizedImages.push_back(resizedImage);
		} else {
			mNewInputSpacing = image->getSpacing();
			resizedImages.push_back(image);
		}
	}
	mRuntimeManager->stopRegularTimer("image input resize");

	return resizedImages;
}


void NeuralNetwork::setTemporalWindow(uint window) {
	if(window < 1) {
        throw Exception("Remember frames has to be > 0.");
	}
	mTemporalWindow = window;
}

void NeuralNetwork::setSignedInputNormalization(bool signedInputNormalization) {
	mSignedInputNormalization = signedInputNormalization;
}

NeuralNetwork::~NeuralNetwork() {
}

void NeuralNetwork::setInputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
	m_engine->addInputNode(portID, name, type, shape);
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::setOutputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
    m_engine->addOutputNode(portID, name, type, shape);
    createOutputPort<DataObject>(portID);
}

void NeuralNetwork::load(std::string filename) {
    if(!fileExists(filename))
        throw FileNotFoundException(filename);
    m_engine->setFilename(filename);
    m_engine->load();
    // Make sure all ports exist
    for(auto node : m_engine->getInputNodes()) {
        createInputPort<DataObject>(node.second.portID);
    }
    for(auto node : m_engine->getOutputNodes()) {
        createOutputPort<DataObject>(node.second.portID);
    }
}

void NeuralNetwork::load(std::vector<uint8_t> model, std::vector<uint8_t> weights) {
    m_engine->setModelAndWeights(model, weights);
    m_engine->load();
    // Make sure all ports exist
    for (auto node : m_engine->getInputNodes()) {
        createInputPort<DataObject>(node.second.portID);
    }
    for (auto node : m_engine->getOutputNodes()) {
        createOutputPort<DataObject>(node.second.portID);
    }
}

void NeuralNetwork::setInferenceEngine(InferenceEngine::pointer engine) {
    m_engine = engine;
}

void NeuralNetwork::setInferenceEngine(std::string engineName) {
    m_engine = InferenceEngineManager::loadEngine(engineName);
    reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
}

InferenceEngine::pointer NeuralNetwork::getInferenceEngine() const {
    return m_engine;
}

void NeuralNetwork::setMeanAndStandardDeviation(float mean, float std) {
    mMean = mean;
    mStd = std;
    mIsModified = true;
}

void NeuralNetwork::setMinAndMaxIntensity(float min, float max) {
    mMinIntensity = min;
    mMaxIntensity = max;
    mMinAndMaxIntensitySet = true;
    mIsModified = true;
}

};
