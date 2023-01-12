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
    
    // If input and/or output nodes names and shapes are defined:
    for(std::string inputOrOutput : {"input", "output"}) {
        auto nodes = getStringListAttribute(inputOrOutput + "-nodes");
        int i = 0;
        for (auto &&info : nodes) {
            auto parts = split(info, ":");
            if(parts.size() > 2)
                throw Exception("Incorrect input-nodes format.");
            NodeType type = NodeType::IMAGE;
            TensorShape shape;
            if(parts.size() == 2) { // Shape was given as well
                auto stringShape = split(parts[1], ",");
                shape.addDimension(-1); // batch
                for (auto strDim : stringShape)
                    shape.addDimension(std::stoi(strDim));
                if (shape.getKnownDimensions() < 3)
                    type = NodeType::TENSOR;
            }
            if(inputOrOutput == "input") {
                setInputNode(i, parts[0], type, shape);
            } else {
                setOutputNode(i, parts[0], type, shape);
            }

            ++i;
        }
    }
    setScaleFactor(getFloatAttribute("scale-factor"));
    setSignedInputNormalization(getBooleanAttribute("signed-input-normalization"));
    setPreserveAspectRatio(getBooleanAttribute("preserve-aspect"));

    // Load network here so that input and output nodes are readily defined after loadAttributes()
    auto format = getModelFormat(getStringAttribute("model"));
    if(!getInferenceEngine()->isModelFormatSupported(format)) {
        reportInfo() << "Model format " << getModelFormatName(format) << " was not supported by engine " << getInferenceEngine()->getName() << ", auto selecting engine which supports this format..." << reportEnd();
        setInferenceEngine(InferenceEngineManager::loadBestAvailableEngine(format));
        reportInfo() << "Selected " << getInferenceEngine()->getName() << " as the best engine for format " << getModelFormatName(format) << reportEnd();
    }
	load(getStringAttribute("model"));
}

void NeuralNetwork::setInputSize(std::string name, std::vector<int> size) {
    mInputSizes[name] = size;
}

NeuralNetwork::NeuralNetwork(std::string modelFilename, std::vector<NeuralNetworkNode> inputNodes,
                             std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                             std::vector<std::string> customPlugins) : NeuralNetwork() {
    mPreserveAspectRatio = false;
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
    if(inferenceEngine.empty()) {
        m_engine = InferenceEngineManager::loadBestAvailableEngine();
        reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
    } else {
        setInferenceEngine(inferenceEngine);
    }
    auto format = getModelFormat(modelFilename);
    if(!m_engine->isModelFormatSupported(format)) {
        reportInfo() << "Model format " << getModelFormatName(format) << " was not supported by engine " << m_engine->getName() << ", auto selecting engine which supports this format..." << reportEnd();
        m_engine = InferenceEngineManager::loadBestAvailableEngine(format);
        reportInfo() << "Selected " << m_engine->getName() << " as the best engine for format " << getModelFormatName(format) << reportEnd();
    }
    for(int i = 0; i < inputNodes.size(); ++i) {
        auto node = inputNodes[i];
        setInputNode(i, node.name, node.type, node.shape);
    }
    for(int i = 0; i < outputNodes.size(); ++i) {
        auto node = outputNodes[i];
        setOutputNode(i, node.name, node.type, node.shape);
    }
    load(modelFilename, customPlugins);
    auto dimOrder = getStringAttribute("dimension-ordering");
    if(!dimOrder.empty()) {
        if(dimOrder == "channel-last") {
            m_engine->setImageOrdering(ImageOrdering::ChannelLast);
        } else if(dimOrder == "channel-first") {
            m_engine->setImageOrdering(ImageOrdering::ChannelFirst);
        } else {
            throw Exception("Incorrect dimension-ordering: " + dimOrder + ". Should be channel-last or channel-first");
        }
    }
}

NeuralNetwork::NeuralNetwork(std::string modelFilename, float scaleFactor, float meanIntensity,
                             float stanardDeviationIntensity,
                             std::vector<NeuralNetworkNode> inputNodes,
                             std::vector<NeuralNetworkNode> outputNodes,
                             std::string inferenceEngine,
                             std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    setScaleFactor(scaleFactor);
    setMeanAndStandardDeviation(meanIntensity, stanardDeviationIntensity);
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
    createStringAttribute("input-nodes", "Input node names, and shapes", "Example: input_node_1:256,256,1  input_node2:1", "");
	createStringAttribute("output-nodes", "Output node names, and shapes", "Example: input_node_1:256,256,1  input_node2:1", "");
	createBooleanAttribute("signed-input-normalization", "Signed input normalization", "Normalize input to -1 and 1 instead of 0 to 1.", false);
    createBooleanAttribute("preserve-aspect", "Preserve aspect ratio of input images", "", mPreserveAspectRatio);
    createStringAttribute("dimension-ordering", "Dimension ordering", "Dimension ordering (channel-last or channel-first), will override auto detecting if set.", "");

	m_engine = InferenceEngineManager::loadBestAvailableEngine();
	reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
}

std::unordered_map<std::string, Tensor::pointer> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, Tensor::pointer> tensors;
    m_batchSize = -1;
    for(auto inputNode : m_engine->getInputNodes()) {
        auto shape = inputNode.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unable to deduce input shape for input node " + inputNode.first + " from network file. "
                            "Either export the file with shape information or supply the input shape manually using setInputNode.");

        if(m_temporalStateNodes.count(inputNode.first) > 0) {
            tensors[inputNode.first] = m_temporalStateNodes[inputNode.first];
            mInputTensors[inputNode.first] = std::vector<Tensor::pointer>{m_temporalStateNodes[inputNode.first]};
            continue;
        }

        std::shared_ptr<DataObject> data = getInputData<DataObject>(inputNode.second.portID);
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
                auto dataList = batch->get();
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
                    auto dataList = sequence->get();
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
                // TODO fix ordering if necessary. We are now assuming that input tensors are in correct ordering.
                mInputTensors[inputNode.first] = inputTensors;
                // We have a list of tensors, convert the list of tensors into a single tensor
                auto shape = inputTensors.front()->getShape();
                m_batchSize = shape[0];
                shape.insertDimension(0, inputTensors.size());
                auto tensor = Tensor::create(shape);
                auto access = tensor->getAccess(ACCESS_READ_WRITE);
                float* data = access->getRawData();
                for(int i = 0; i < inputTensors.size(); ++i) {
                    auto accessRead = inputTensors[i]->getAccess(ACCESS_READ);
                    const int totalSize = accessRead->getShape().getTotalSize();
                    std::memcpy(&data[i*totalSize], accessRead->getRawData(), totalSize*sizeof(float));
                }
            }
        } else {
            // TODO fix ordering if necessary. We are now assuming that input tensors are in correct ordering.
            // Input is a tensor, no conversion needed
            tensors[inputNode.first] = tensor;
            mInputTensors[inputNode.first].push_back(tensor);
        }
        mRuntimeManager->stopRegularTimer("input_processing");
	}

	return tensors;
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
inline int getPosition(int x, int nrOfClasses, int j, int size, ImageOrdering ordering) {
    return ordering == ImageOrdering::ChannelLast ? x*nrOfClasses + j : x + j*size;
}

Tensor::pointer NeuralNetwork::standardizeOutputTensorData(Tensor::pointer tensor, int sample) {
    // Transform tensor to channel last if necessary
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
        // Convert to channel last
        const int nrOfClasses = tensor->getShape()[0];
        const int size = tensor->getShape().getTotalSize()/nrOfClasses;
        auto newTensorData = make_uninitialized_unique<float[]>(size*nrOfClasses);
        auto tensorAccess = tensor->getAccess(ACCESS_READ);
        const float* tensorData = tensorAccess->getRawData();
        // TODO move to GPU (if tensor is large)
        for(int x = 0; x < size; ++x) {
            for(uchar j = 0; j < nrOfClasses; ++j) {
                newTensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelLast)] = tensorData[getPosition(x, nrOfClasses, j, size, ImageOrdering::ChannelFirst)];
            }
        }
        auto oldShape = tensor->getShape();
        TensorShape newShape;
        for(int i = 1; i < oldShape.getDimensions(); ++i)
            newShape.addDimension(oldShape[i]);
        newShape.addDimension(oldShape[0]);
        auto newTensor = Tensor::create(std::move(newTensorData), newShape);
        newTensor->setSpacing(tensor->getSpacing());
        tensor = newTensor;
    }

    // Transfer frame data and spacing information from input to output data
    for(auto& inputNode : m_engine->getInputNodes()) {
        if(mInputImages.count(inputNode.first) > 0) {
            for(auto &&frameData : mInputImages[inputNode.first][sample]->getFrameData())
                tensor->setFrameData(frameData.first, frameData.second);
            for(auto &&lastFrame : mInputImages[inputNode.first][sample]->getLastFrame())
                tensor->setLastFrame(lastFrame);
            // TODO will cause issue if multiple input images:
            tensor->setSpacing(mNewInputSpacing);
            tensor->setFrameData("network-input-size-x", std::to_string(m_newInputSize.x()));
            tensor->setFrameData("network-input-size-y", std::to_string(m_newInputSize.y()));
            tensor->setFrameData("network-input-size-z", std::to_string(m_newInputSize.z()));
            SceneGraph::setParentNode(tensor, mInputImages[inputNode.first][sample]);
        } else {
            for(auto &&frameData : mInputTensors[inputNode.first][sample]->getFrameData())
                tensor->setFrameData(frameData.first, frameData.second);
            for(auto &&lastFrame : mInputTensors[inputNode.first][sample]->getLastFrame())
                tensor->setLastFrame(lastFrame);
            SceneGraph::setParentNode(tensor, mInputTensors[inputNode.first][sample]);
        }
    }
    return tensor;
}

void NeuralNetwork::runNeuralNetwork() {
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

    processOutputTensors();
}

void NeuralNetwork::processOutputTensors() {
    mRuntimeManager->startRegularTimer("output_processing");
    // Collect output data of network and add to output ports
    for(const auto &node : m_engine->getOutputNodes()) {
        // TODO if input was a batch, the output should be converted to a batch as well
        // TODO and any frame data (such as patch info should be transferred)
        auto tensor = m_engine->getOutputData(node.first);

        if(m_temporalStateNodes.count(node.first) > 0) {
            m_temporalStateNodes[node.first] = tensor;
            continue;
        }

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
                auto newData = make_uninitialized_unique<float[]>(size);
                std::memcpy(newData.get(), &(rawTensorData[i*size]), size*sizeof(float));
                auto newTensor = Tensor::create(std::move(newData), newShape);
                newTensor = standardizeOutputTensorData(newTensor, i);
                tensorList.push_back(newTensor);
            }
            auto outputBatch = Batch::create(tensorList);
            m_processedOutputData[node.second.portID] = outputBatch;
        } else {
            // Remove first dimension as it is 1, due to batch size 1
            tensor->deleteDimension(0);
            tensor = standardizeOutputTensorData(tensor);
            m_processedOutputData[node.second.portID] = tensor;
        }
    }
    // Copy any temporal state links
    for(auto link : m_temporalStateLinks) {
        m_temporalStateNodes[link.first] = m_temporalStateNodes[link.second];
    }
    mRuntimeManager->stopRegularTimer("output_processing");
}

void NeuralNetwork::execute() {
    // Load, prepare input and run network
    runNeuralNetwork();

    // Add output data to output ports
    for(const auto &node : m_processedOutputData) {
        addOutputData(node.first, node.second);
    }
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

    auto tensor = Tensor::create(std::move(values), shape);
    return tensor;
}

std::vector<std::shared_ptr<Image>> NeuralNetwork::resizeImages(const std::vector<std::shared_ptr<Image>> &images, int width, int height, int depth) {
    m_newInputSize = Vector3i(width, height, depth);
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
            auto resizedImage = resizer->updateAndGetOutputData<Image>();
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
    if(m_engine->isLoaded())
        throw Exception("NeuralNetwork setInputNode/setOutputNode must be called before load()");
	m_engine->addInputNode(portID, name, type, shape);
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::setOutputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
    if(m_engine->isLoaded())
        throw Exception("NeuralNetwork setInputNode/setOutputNode must be called before load()");
    m_engine->addOutputNode(portID, name, type, shape);
    createOutputPort<DataObject>(portID);
}

void NeuralNetwork::load(std::string filename, std::vector<std::string> customPlugins) {
    if(!fileExists(filename))
        throw FileNotFoundException(filename);

    if(!customPlugins.empty())
        m_engine->loadCustomPlugins(customPlugins);
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

void NeuralNetwork::load(std::vector<uint8_t> model, std::vector<uint8_t> weights, std::vector<std::string> customPlugins) {
    if(!customPlugins.empty())
        m_engine->loadCustomPlugins(customPlugins);
    m_engine->setModelAndWeights(model, weights);
    m_engine->load();
    // Make sure all ports exist
    for(auto node : m_engine->getInputNodes()) {
        createInputPort<DataObject>(node.second.portID);
    }
    for(auto node : m_engine->getOutputNodes()) {
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

std::map<std::string, NeuralNetworkNode> NeuralNetwork::getInputNodes() {
    auto nodes = m_engine->getInputNodes();
    // TODO Use same node data type as InferenceEngine?
    std::map<std::string, NeuralNetworkNode> result;
    for(auto& node : nodes) {
        result.insert(std::make_pair(node.first, NeuralNetworkNode(node.first, node.second.type, node.second.shape, node.second.portID)));
    }
    return result;
}

std::map<std::string, NeuralNetworkNode> NeuralNetwork::getOutputNodes() {
    auto nodes = m_engine->getOutputNodes();
    // TODO Use same node data type as InferenceEngine?
    std::map<std::string, NeuralNetworkNode> result;
    for(auto& node : nodes) {
        result.insert(std::make_pair(node.first, NeuralNetworkNode(node.first, node.second.type, node.second.shape, node.second.portID)));
    }
    return result;
}

NeuralNetworkNode NeuralNetwork::getNode(std::string name) {
    {
        auto nodes = m_engine->getInputNodes();
        if(nodes.count(name) > 0) {
            return NeuralNetworkNode(name, nodes[name].type, nodes[name].shape, nodes[name].portID);
        }
    }
    {
        auto nodes = m_engine->getOutputNodes();
        if(nodes.count(name) > 0) {
            return NeuralNetworkNode(name, nodes[name].type, nodes[name].shape, nodes[name].portID);
        }
    }
    throw Exception("Node with name " + name + " not found in neural network");
}

void NeuralNetwork::addTemporalState(std::string inputNodeName, std::string outputNodeName, TensorShape shape) {
    bool found = false;
    for(auto&& port : m_engine->getInputNodes()) {
        if(port.first == inputNodeName) {
            mInputPorts.erase(port.second.portID);
            found = true;
            break;
        }
    }
    if(!found)
        throw Exception("Could not find the input node with name " + inputNodeName + " given to addTemporalState()");
    found = false;
    for(auto&& port : m_engine->getOutputNodes()) {
        if(port.first == outputNodeName) {
            mOutputPorts.erase(port.second.portID);
            found = true;
            break;
        }
    }
    if(!found)
        throw Exception("Could not find the output node with name " + outputNodeName + " given to addTemporalState()");

    // TODO reset port numbers? In case where main input gets an input number other than 0?

    auto node = m_engine->getInputNode(inputNodeName);

    // TODO Support batch size other than 1?
    node.shape[0] = 1;
    auto zeros = std::make_unique<float[]>(node.shape.getTotalSize());
    auto tensor = Tensor::create(std::move(zeros), node.shape);

    m_temporalStateNodes[inputNodeName] = tensor;
    m_temporalStateNodes[outputNodeName] = nullptr;
    m_temporalStateLinks.push_back(std::make_pair(inputNodeName, outputNodeName));

    setModified(true);
}

Sequence::Sequence(std::vector<std::shared_ptr<Image>> images) {
    m_data = InferenceDataList(std::move(images));
}
Sequence::Sequence(std::vector<std::shared_ptr<Tensor>> tensors) {
    m_data = InferenceDataList(std::move(tensors));
}

Batch::Batch(std::vector<std::shared_ptr<Image>> images) {
    m_data = InferenceDataList(std::move(images));
}
Batch::Batch(std::vector<std::shared_ptr<Tensor>> tensors) {
    m_data = InferenceDataList(std::move(tensors));
}

};
