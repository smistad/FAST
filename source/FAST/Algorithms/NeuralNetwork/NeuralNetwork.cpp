#include "NeuralNetwork.hpp"
// Windows hack for removing need for protobuf
#ifdef WIN32
#include <google/protobuf/stubs/logging.h>
#undef GOOGLE_LOG_IF
#define GOOGLE_LOG_IF(LEVEL, CONDITION) \
  !(CONDITION) ? std::clog : std::cerr
// end hack
#endif
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Tensor.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include <tensorflow/core/framework/step_stats.pb.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/types.pb.h>
#include <tensorflow/core/lib/strings/stringprintf.h>
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/platform/mutex.h>
#include <tensorflow/core/platform/types.h>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/graph/default_device.h>
#include <tensorflow/core/platform/init_main.h>
#include <tensorflow/cc/framework/ops.h>
#include <tensorflow/core/platform/logging.h>



namespace fast {

/**
 * This specialized Tensor Data class, allow us to store Tensorflow type tensors as FAST tensors.
 */
class TensorflowTensor : public Tensor {
    FAST_OBJECT(TensorflowTensor)
    public:
        void create(tensorflow::Tensor&& tensorflowTensor);
        TensorAccess::pointer getAccess(accessType access) override;
    private:
        tensorflow::Tensor m_tensorflowTensor;
};

void TensorflowTensor::create(tensorflow::Tensor&& tensorflowTensor) {
    m_tensorflowTensor = tensorflowTensor;
    auto shape = m_tensorflowTensor.shape();
    for(int i = 0; i < shape.dims(); ++i)
        m_shape.addDimension(shape.dim_size(i));
}

TensorAccess::pointer TensorflowTensor::getAccess(accessType type) {
    // TODO process type
    return std::make_unique<TensorAccess>(m_tensorflowTensor.flat<float>().data(), m_shape, std::static_pointer_cast<Tensor>(mPtr.lock()));
}

static TensorShape getShape(const tensorflow::NodeDef& node) {
    TensorShape resultShape;
    if(node.attr().count("shape") > 0) {
        auto shape = node.attr().at("shape").shape();
        for(int i = 0; i < shape.dim_size(); i++) {
            resultShape.addDimension(shape.dim(i).size());
        }
    }
    return resultShape;
}

void NeuralNetwork::load(std::string networkFilename) {

	tensorflow::SessionOptions options;
	tensorflow::ConfigProto &config = options.config;
	tensorflow::GPUOptions* gpuOptions = config.mutable_gpu_options();
	gpuOptions->set_allow_growth(true); // Set this so that tensorflow will not use up all GPU memory
	//gpuOptions->set_per_process_gpu_memory_fraction(0.5);
	mSession.reset(tensorflow::NewSession(options));
	tensorflow::GraphDef tensorflow_graph;

	{
		reportInfo() << "Loading network file: " << networkFilename << reportEnd();
		tensorflow::Status s = ReadBinaryProto(tensorflow::Env::Default(), networkFilename, &tensorflow_graph);
		if (!s.ok()) {
			throw Exception("Could not read TensorFlow graph file " + networkFilename);
		}
	}

	bool nodesSpecified = true;
    int inputCounter = 0;
	if(mInputNodes.size() == 0) {
		nodesSpecified = false;
	}
    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
		if(mInputNodes.count(node.name()) > 0) {
		}
		if(node.op() == "Placeholder") {
			if(node.name().find("keras_learning_phase") != std::string::npos) {
				mLearningPhaseTensors.push_back(node.name());
			} else {
				// Input node found:
				// Get its shape
				// Input nodes use the Op Placeholder
				reportInfo() << "Found input node: " << i << " with name " << node.name() << reportEnd();
				auto shape = getShape(node);
				reportInfo() << "Node has shape " << shape.toString() << reportEnd();
				if(mInputNodes.count(node.name()) == 0) {
					if(nodesSpecified) {
						throw Exception("Encountered unknown node " + node.name());
					}
					reportInfo() << "Node was not specified by user" << reportEnd();
					// If node has not been specified by user, we need to add it
					// and thus know its type (fast image or tensor)
					// It is assumed to be an image if input shape has at least 4 dimensions
					NodeType type = NodeType::TENSOR;
					if(shape.getKnownDimensions() >= 2) {
						reportInfo() << "Assuming node is an image" << reportEnd();
						type = NodeType::IMAGE;
					} else {
						reportInfo() << "Assuming node is a tensor" << reportEnd();
					}
					addInputNode(inputCounter, tensorflow_graph.node(0).name(), type, shape);
					++inputCounter;
				}

				// Set its shape
				mInputNodes[node.name()].shape = shape;
			}
		}
	}

	reportInfo() << "Creating session." << reportEnd();
	tensorflow::Status s = mSession->Create(tensorflow_graph);
	if (!s.ok()) {
		throw Exception("Could not create TensorFlow Graph");
	}

	//tensorflow::graph::SetDefaultDevice("/gpu:0", &tensorflow_graph);

	// Clear the proto to save memory space.
	tensorflow_graph.Clear();
	reportInfo() << "TensorFlow graph loaded from: " << networkFilename << reportEnd();

	mModelLoaded = true;
}

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
	mModelLoaded = false;
	mPreserveAspectRatio = false;
	mScaleFactor = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
	createStringAttribute("model", "Model path", "Path to neural network tensorflow model", "");
	createIntegerAttribute("input_size", "Input size", "Image input size", 128);
	createFloatAttribute("scale_factor", "Scale factor", "Scale factor", mScaleFactor);
	createStringAttribute("output_names", "Output names", "Name of output nodes", "");
	createBooleanAttribute("signed_input_normalization", "Signed input normalization", "Normalize input to -1 and 1 instead of 0 to 1.", false);
}

std::unordered_map<std::string, Tensor::pointer> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, Tensor::pointer> tensors;
    int batchSize = -1;
    for(auto inputNode : mInputNodes) {
        auto shape = inputNode.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unable to deduce input shape from .pb file. Either export the file with shape information or supply the input shape manually using setInputShape");

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
            int depth = 1;
            int timesteps = 0;
            if(containsSequence) {
                // Temporal input
                timesteps = shape[1];
                if(dims == 6) // 3D
                    depth = shape[2];
            } else {
                if(dims == 5) // 3D
                    depth = shape[1];
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

void NeuralNetwork::execute() {
	// Add output tensors to the output of this PO
	auto inputTensors = processInputData();
	auto output = executeNetwork(inputTensors);
    for(auto outputTensor : output) {
        auto node = outputTensor.first;
        auto tensor = outputTensor.second;
        addOutputData(node.portID, tensor);
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
    const int channels = shape[dims-1];
    const int width = shape[dims-2];
    const int height = shape[dims-3];
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
        depth = shape[dims-4];
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
        kernel.setArg(3, (int) (mSignedInputNormalization ? 1 : 0));
        cl::NDRange globalSize;
        if(image->getDimensions() == 2) {
            kernel.setArg(0, *access->get2DImage());
            kernel.setArg(4, (int) (mHorizontalImageFlipping ? 1 : 0));
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

std::vector<std::pair<NeuralNetwork::NetworkNode, Tensor::pointer>> NeuralNetwork::executeNetwork(std::unordered_map<std::string, Tensor::pointer> tensors) {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");
	if(mOutputNodes.empty())
		throw Exception("At least one output node has to be given to the NeuralNetwork before execution");

	// For each input, create a tensorflow tensor:
	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors;
	for(auto inputNode : mInputNodes) {
		const std::string name = inputNode.first;
		auto shape = tensors[name]->getShape();
		if(shape.getUnknownDimensions() > 0)
		    throw Exception("Input shape must be fully known when executing NN");

		// Construct tensorflow tensor
        tensorflow::TensorShape tensorShape;
        for(auto i : shape.getAll()) {
            tensorShape.AddDim(i);
        }
        tensorflow::Tensor input_tensor(
                tensorflow::DT_FLOAT,
                tensorShape
        );

        // Give tensor data to tensorflow
        // TODO is the data here actually moved?
        TensorAccess::pointer access = tensors[name]->getAccess(ACCESS_READ);
        switch(shape.getDimensions()) {
            case 2:
                input_tensor.tensor<float, 2>() = std::move(access->getData<2>());
                break;
            case 3:
                input_tensor.tensor<float, 3>() = std::move(access->getData<3>());
                break;
            case 4:
                input_tensor.tensor<float, 4>() = std::move(access->getData<4>());
                break;
            case 5:
                input_tensor.tensor<float, 5>() = std::move(access->getData<5>());
                break;
			case 6:
				input_tensor.tensor<float, 6>() = std::move(access->getData<6>());
				break;
            default:
                throw Exception("Invalid tensor dimension size");
		}

		// Add tensorflow tensor to list of input tensors
		input_tensors.push_back(std::make_pair(name, input_tensor));
	}

    for(std::string name : mLearningPhaseTensors) {
        // Create a scalar tensor which tells the system we are NOT doing training
        tensorflow::Tensor input_tensor2(
                tensorflow::DT_BOOL,
                tensorflow::TensorShape() // Scalar
        );
        auto input_tensor_mapped2 = input_tensor2.tensor<bool, 0>();
        input_tensor_mapped2(0) = false;
        input_tensors.push_back(std::make_pair(name, input_tensor2));
    }

	std::vector<tensorflow::Tensor> output_tensors;

	reportInfo() << "Running network" << reportEnd();
	tensorflow::Status s;
	mRuntimeManager->startRegularTimer("network_execution");
	std::vector<std::string> outputNames;
	for(auto node : mOutputNodes)
	    outputNames.push_back(node.first);
	s = mSession->Run(input_tensors, outputNames, {}, &output_tensors);
	mRuntimeManager->stopRegularTimer("network_execution");

	if (!s.ok()) {
		throw Exception("Error during inference: " + s.ToString());
	}
	reportInfo() << "Finished executing network" << reportEnd();

    // Collect all output data as FAST tensors
    std::vector<std::pair<NetworkNode, Tensor::pointer>> result;
    for(int j = 0; j < outputNames.size(); ++j) {
        const std::string outputName = outputNames[j];
        const NetworkNode node = mOutputNodes[outputName];

        auto tensor = TensorflowTensor::New();
        tensor->create(std::move(output_tensors[j]));
        result.push_back(std::make_pair(node, tensor));
	}
	reportInfo() << "Finished parsing output" << reportEnd();

    return result;
}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images, int width, int height, int depth) {
    mRuntimeManager->startRegularTimer("image input resize");
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(width != image->getWidth() || height != image->getHeight() || depth != image->getDepth()) {
			// Only resize if needed
            ImageResizer::pointer resizer = ImageResizer::New();
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
	load(getStringAttribute("model"));
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
	if(mSession) {
		mSession->Close();
	}
}

void NeuralNetwork::addInputNode(uint portID, std::string name, NeuralNetwork::NodeType type, TensorShape shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mInputNodes[name] = node;
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::addOutputNode(uint portID, std::string name, NeuralNetwork::NodeType type, TensorShape shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mOutputNodes[name] = node;
	createOutputPort<DataObject>(portID);
}

};
