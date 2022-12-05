#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/Algorithms/ImageAdd/ImageAdd.hpp>
#include <FAST/Algorithms/IntensityNormalization/IntensityNormalization.hpp>
#include <FAST/Algorithms/IntensityClipping/IntensityClipping.hpp>
#include <FAST/Algorithms/ImageCaster/ImageCaster.hpp>
#include "ImageToImageNetwork.hpp"
#include "TensorToImage.hpp"

namespace fast {

void ImageToImageNetwork::loadAttributes() {
	NeuralNetwork::loadAttributes();

    setScaleFactor(1.0f);
    mScaleFactor2 = getFloatAttribute("scale-factor");
    setResidualNetwork(getBooleanAttribute("residual-network"));
    setCastOutput(getBooleanAttribute("cast-output"));
    setResizeOutput(getBooleanAttribute("resize-output"));
    setIterations(getIntegerAttribute("iterations"));
    setChannels(getIntegerListAttribute("channels"));
}

void ImageToImageNetwork::init() {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");

    m_tensorToImage = TensorToImage::New();

    createBooleanAttribute("residual-network", "Residual network", "Whether the neural network is a residual network", m_residualNetwork);
    createBooleanAttribute("resize-output", "Resize output image", "Resize output image to original size", m_resizeBackToOriginalSize);
    createBooleanAttribute("cast-output", "Cast output image", "Cast output image to original type", m_castBackToOriginalType);
    createIntegerAttribute("iterations", "Iterations", "How many iterations to run the neural network", m_iterations);
    createIntegerAttribute("channels", "Channels", "Output image channels to extract from neural network", 0);
}

ImageToImageNetwork::ImageToImageNetwork(std::string modelFilename,
                                         float scaleFactor,
                                         int iterations,
                                         bool residualNetwork,
                                         bool resizeBackToOriginalSize,
                                         bool castBackToOriginalType,
                                         std::vector<int> channelsToExtract,
                                         float meanIntensity,
                                         float stanardDeviationIntensity, std::vector<NeuralNetworkNode> inputNodes,
                                         std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                                         std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, scaleFactor, meanIntensity, stanardDeviationIntensity, inputNodes, outputNodes,inferenceEngine,customPlugins) {
    init();

    setIterations(iterations);
    setScaleFactor(1.0f);
    mScaleFactor2 = scaleFactor;
    setResidualNetwork(residualNetwork);
    setResizeOutput(resizeBackToOriginalSize);
    setCastOutput(castBackToOriginalType);
    setChannels(channelsToExtract);
}

ImageToImageNetwork::ImageToImageNetwork(std::string modelFilename, std::vector<NeuralNetworkNode> inputNodes,
                                         std::vector<NeuralNetworkNode> outputNodes, std::string inferenceEngine,
                                         std::vector<std::string> customPlugins) : NeuralNetwork(modelFilename, inputNodes, outputNodes, inferenceEngine, customPlugins) {
    init();
    m_tensorToImage = TensorToImage::New();
}

ImageToImageNetwork::ImageToImageNetwork() {
    init();
    m_tensorToImage = TensorToImage::New();
}

void ImageToImageNetwork::execute() {
    // Check if network is loaded, if not do it
    if(!m_engine->isLoaded())
        m_engine->load();
    if(!m_channelsToExtract.empty())
        m_tensorToImage->setChannels(m_channelsToExtract);

    // Assumes one input and one output

    // Prepare input data
    auto inputTensors = processInputData();

    if(m_engine->getInputNodes().size() != 1)
        throw Exception("ImageToImageNetwork can only handle single input networks atm.");

    auto inputNodes = m_engine->getInputNodes();
    auto inputNode = inputNodes.begin();

    Image::pointer inputImage = getInputData<Image>();
    DataType inputDataType = inputImage->getDataType();
    // Normalize the input
    auto normalizer = IntensityNormalization::create(0.0f, 1.0f, 0.0f, 1.0f / mScaleFactor2)
            ->connect(inputImage);
    inputImage = normalizer->runAndGetOutputData<Image>();
    auto shape = inputNode->second.shape;
    const int dims = shape.getDimensions();
    int height = shape[dims - 3];
    int width = shape[dims - 2];
    if(m_engine->getPreferredImageOrdering() == ImageOrdering::ChannelFirst) {
        height = shape[dims - 2];
        width = shape[dims - 1];
    }
    auto resizedImages = resizeImages({inputImage}, width, height, 1);
    inputImage = resizedImages[0];
    Image::pointer image = inputImage;
    for(int i = 0; i < m_iterations; ++i) {
        auto tensor = convertImagesToTensor({inputImage}, shape, false);
        // Give input tensor to inference engine
        m_engine->setInputData(inputNode->first, tensor);

        // Run network
        mRuntimeManager->startRegularTimer("inference");
        m_engine->run();
        mRuntimeManager->stopRegularTimer("inference");

        // Process output tensors from engine
        processOutputTensors();

        // Convert tensor to image
        auto data = m_processedOutputData[0];
        m_tensorToImage->connect(data);
        image = m_tensorToImage->runAndGetOutputData<Image>();

        if(m_residualNetwork) {
            // If this is a residual learning network, we have to add the input and the output to create a new image
            auto imageAdd = ImageAdd::create()->connect(0, inputImage)->connect(1, image);
            image = imageAdd->runAndGetOutputData<Image>();
        }
        // TODO Make normalization/clipping optional
        auto imageClip = IntensityClipping::create(0.0f, 1.0f)->connect(image);
        image = imageClip->runAndGetOutputData<Image>();
        inputImage = image;
    }

    mRuntimeManager->startRegularTimer("tensor_to_image");
    // Convert tensor to image
    if(m_castBackToOriginalType) {
        auto caster = ImageCaster::create(inputDataType, 1.0f/mScaleFactor2)->connect(image);
        image = caster->runAndGetOutputData<Image>();
    }
    if(m_resizeBackToOriginalSize) {
        auto resizer = ImageResizer::New();
        resizer->setInputData(image);
        resizer->setSize(mInputImages.begin()->second[0]->getSize().cast<int>());
        resizer->setInterpolation(true);
        image = resizer->updateAndGetOutputData<Image>();
    }
    addOutputData(0, image);

    mRuntimeManager->stopRegularTimer("tensor_to_image");
}

void ImageToImageNetwork::setIterations(int iterations) {
    if(iterations < 0)
        throw Exception("ImageToImageNetwork iterations must be >= 0");

    m_iterations = iterations;
    setModified(true);
}

int ImageToImageNetwork::getIterations() const {
    return m_iterations;
}

void ImageToImageNetwork::setResidualNetwork(bool residual) {
    m_residualNetwork = residual;
    setModified(true);
}

void ImageToImageNetwork::setResizeOutput(bool resizeOutput) {
    m_resizeBackToOriginalSize = resizeOutput;
    setModified(true);
}

void ImageToImageNetwork::setCastOutput(bool castOutput) {
    m_castBackToOriginalType = castOutput;
    setModified(true);
}

void ImageToImageNetwork::setChannels(std::vector<int> channels) {
    m_channelsToExtract = channels;
    setModified(true);
}

}