#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Tensor.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "InferenceEngineList.hpp"


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

NeuralNetwork::NeuralNetwork() {
	mPreserveAspectRatio = false;
	mScaleFactor = 1.0f;
	mMean = 0.0;
	mStd = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
	createStringAttribute("model", "Model path", "Path to neural network tensorflow model", "");
	createIntegerAttribute("input_size", "Input size", "Image input size", 128);
	createFloatAttribute("scale_factor", "Scale factor", "Scale factor", mScaleFactor);
	createStringAttribute("output_names", "Output names", "Name of output nodes", "");
	createBooleanAttribute("signed_input_normalization", "Signed input normalization", "Normalize input to -1 and 1 instead of 0 to 1.", false);

	m_engine = InferenceEngineRegistry::createPreferredEngine();
	reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
}

std::unordered_map<std::string, Tensor::pointer> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, Tensor::pointer> tensors;
    int batchSize = -1;
    for(auto inputNode : m_engine->getInputNodes()) {
        auto shape = inputNode.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unable to deduce input shape from network file. "
                            "Either export the file with shape information or supply the input shape manually using addInputNode.");

        SharedPointer<DataObject> data = getInputData<DataObject>(inputNode.second.portID);

        bool containsSequence = false;
        // Check if data object is an tensor by doing a dynamic cast
        Tensor::pointer tensor = std::dynamic_pointer_cast<Tensor>(data);
        if(!tensor) {
            if(shape.getDimensions() < 4)
                throw Exception("Trying to attach an image to an input node with shape with fewer than 4 dimensions");

            // If not a tensor, data is either a Batch of images or a single Image
            Batch::pointer batch = std::dynamic_pointer_cast<Batch>(data);
            std::vector<Image::pointer> inputImages;
            if(batch) {
                Batch::access access = batch->getAccess(ACCESS_READ);
                inputImages = access->getData();
                if(batchSize == -1) {
                    batchSize = inputImages.size();
                } else {
                    throw Exception("Inconsistent batch size accross input nodes");
                }
            } else {
                batchSize = 1;
                Sequence::pointer sequence = std::dynamic_pointer_cast<Sequence>(data);
                if(sequence) {
                    Sequence::access access = sequence->getAccess(ACCESS_READ);
                    inputImages = access->getData();
                    containsSequence = true;
                    if(shape[1] == -1) {
                        // Nr of timesteps unknown in the shape, set it
                        shape[1] = inputImages.size();
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
                            throw Exception("err");
                    } else {
                        inputImages = {image};
                    }
                }
            }
            mInputImages[inputNode.first] = inputImages;

            // Resize images to fit input
            const int dims = shape.getDimensions();
            int height = shape[dims-3];
            int width = shape[dims-2];
            if(m_engine->getPreferredImageOrdering() == ImageOrdering::CHW) {
                height = shape[dims-2];
                width = shape[dims-1];
            }
            int depth = 1;
            int timesteps = 0;
            if(containsSequence) {
                // Temporal input
                timesteps = shape[1];
                if(dims == 6) // 3D
                    depth = m_engine->getPreferredImageOrdering() == ImageOrdering::HWC ? shape[dims-4] : shape[dims-3];
            } else {
                if(dims == 5) // 3D
                    depth = m_engine->getPreferredImageOrdering() == ImageOrdering::HWC ? shape[dims-4] : shape[dims-3];
            }
            inputImages = resizeImages(inputImages, width, height, depth);

			std::vector<Tensor::pointer> inputTensors;
            // Convert images to tensors
            shape[0] = batchSize;
			tensors[inputNode.first] = convertImagesToTensor(inputImages, shape, containsSequence);
        } else {
            tensors[inputNode.first] = tensor;
        }
	}

	return tensors;
}

void NeuralNetwork::run() {
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
	m_engine->run();
}

void NeuralNetwork::execute() {

    // Load, prepare input and run network
    run();

	// Collect output data of network and add to output ports
    for(const auto &node : m_engine->getOutputNodes()) {
        addOutputData(node.second.portID, m_engine->getOutputData(node.first));
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
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::CHW) {
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
        if(m_engine->getPreferredImageOrdering() == ImageOrdering::CHW) {
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
        cl::NDRange globalSize;
        if(image->getDimensions() == 2) {
            kernel.setArg(0, *access->get2DImage());
            kernel.setArg(6, (int) (mHorizontalImageFlipping ? 1 : 0));
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
        device->getCommandQueue().enqueueReadBuffer(buffer, CL_FALSE, 0, sizeof(float) * size,
                                                    values.get() + i*size);
    }
    device->getCommandQueue().finish();

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
			DataPort::pointer port = resizer->getOutputPort();
            resizer->update(0);
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

void NeuralNetwork::loadAttributes() {
	m_engine->setFilename(getStringAttribute("model"));
	std::vector<int> inputSize = getIntegerListAttribute("input_size");
	// TODO Fix
	//setInputSize(inputSize.at(0), inputSize.at(1));
	std::vector<std::string> outputNames = getStringListAttribute("output_names");
	//setOutputParameters(outputNames);
	setScaleFactor(getFloatAttribute("scale_factor"));
	setSignedInputNormalization(getBooleanAttribute("signed_input_normalization"));
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

void NeuralNetwork::addInputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
	m_engine->addInputNode(portID, name, type, shape);
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::addOutputNode(uint portID, std::string name, NodeType type, TensorShape shape) {
    m_engine->addOutputNode(portID, name, type, shape);
    createOutputPort<DataObject>(portID);
}

void NeuralNetwork::load(std::string filename) {
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

void NeuralNetwork::setInferenceEngine(InferenceEngine::pointer engine) {
    m_engine = engine;
}

void NeuralNetwork::setInferenceEngine(std::string engineName) {
    auto list = InferenceEngineRegistry::getList();
    if(list.count(engineName) == 0) {
        std::string nameList;
        for(auto ie : list)
            nameList += ie + " ";
        throw Exception("The inference engine " + engineName + " was not available. Available inference engines are:" + nameList);
    }
    m_engine = InferenceEngineRegistry::create(engineName);
    reportInfo() << "Inference engine " << m_engine->getName() << " selected" << reportEnd();
}

InferenceEngine::pointer NeuralNetwork::getInferenceEngine() const {
    return m_engine;
}

void NeuralNetwork::setMeanAndStandardDeviation(float mean, float std) {
    mMean = mean;
    mStd = std;
}

};
