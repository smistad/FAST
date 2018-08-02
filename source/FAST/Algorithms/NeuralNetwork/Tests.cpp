#include <FAST/Testing.hpp>
#include "NeuralNetwork.hpp"
#include "PixelClassifier.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>

using namespace fast;

TEST_CASE("Execute NN on single 2D image", "[fast][neuralnetwork]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");

    auto segmentation = PixelClassifier::New();
    segmentation->setNrOfClasses(3);
    segmentation->load(Config::getTestDataPath() + "NeuralNetworkModels/jugular_vein_segmentation.pb");
    segmentation->setScaleFactor(1.0f / 255.0f);
    segmentation->addOutputNode(0, "conv2d_23/truediv");
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->enableRuntimeMeasurements();

    auto segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(segmentation->getOutputPort());
    segmentationRenderer->setOpacity(0.25);
    segmentationRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color::Red());
    segmentationRenderer->setColor(Segmentation::LABEL_BLOOD, Color::Blue());

    auto imageRenderer = ImageRenderer::New();
    imageRenderer->setInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
    window->setTimeout(1000);
    window->start();
}

TEST_CASE("Multi input, single output network", "[fast][neuralnetwork]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/multi_input_single_output.pb");
    network->addOutputNode(0, "dense/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->setInputConnection(0, importer->getOutputPort());
    network->setInputConnection(1, importer->getOutputPort());
    auto port = network->getOutputPort();
    network->update(0);
    auto data = port->getNextFrame<Tensor>();
    // We are expecting a tensor output with dimensions (1, 6)
    REQUIRE(data->getShape().getDimensions() == 2);
    CHECK(data->getShape()[1] == 6);
}

TEST_CASE("Single input, multi output network", "[fast][neuralnetwork]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/single_input_multi_output.pb");
    network->addOutputNode(0, "dense_1/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->addOutputNode(1, "dense_2/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->setInputConnection(0, importer->getOutputPort());
    auto port1 = network->getOutputPort(0);
    auto port2 = network->getOutputPort(1);
    network->update(0);
    auto data1 = port1->getNextFrame<Tensor>();
    auto data2 = port2->getNextFrame<Tensor>();

    // We are expecting two tensors as output each with dimensions (1, 6)
    REQUIRE(data1->getShape().getDimensions() == 2);
    CHECK(data1->getShape()[0] == 1);
    CHECK(data1->getShape()[1] == 6);
    REQUIRE(data2->getShape().getDimensions() == 2);
    CHECK(data2->getShape()[0] == 1);
    CHECK(data2->getShape()[1] == 6);
}

TEST_CASE("Single 3D image input network", "[fast][neuralnetwork][3d]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/single_volume_input.pb");
    network->addOutputNode(0, "dense/BiasAdd");
    network->setInputConnection(0, importer->getOutputPort());
    auto port = network->getOutputPort();
    network->update(0);
    auto data = port->getNextFrame<Tensor>();

    // We are expecting one tensor as output with shape (1, 10)
    REQUIRE(data->getShape().getDimensions() == 2);
    CHECK(data->getShape()[0] == 1);
    CHECK(data->getShape()[1] == 10);
}

TEST_CASE("Execute NN on batch of 2D images", "[fast][neuralnetwork]") {
    std::vector<Image::pointer> images;

    // Import data
    {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
        auto port = importer->getOutputPort();
        importer->update(0);
        auto data = port->getNextFrame<Image>();
        images.push_back(data);
        images.push_back(data);
    }
    auto batch = Batch::New();
    batch->create(images);

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/single_input_multi_output.pb");
    network->addOutputNode(0, "dense_1/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->addOutputNode(1, "dense_2/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->setInputData(batch);
    auto port1 = network->getOutputPort(0);
    auto port2 = network->getOutputPort(1);
    network->update(0);
    auto data1 = port1->getNextFrame<Tensor>();
    auto data2 = port2->getNextFrame<Tensor>();

    // We are expecting two tensors as output each with dimensions (1, 6)
    REQUIRE(data1->getShape().getDimensions() == 2);
    CHECK(data1->getShape()[0] == 2);
    CHECK(data1->getShape()[1] == 6);
    REQUIRE(data2->getShape().getDimensions() == 2);
    CHECK(data2->getShape()[0] == 2);
    CHECK(data2->getShape()[1] == 6);
}

TEST_CASE("NN: temporal input static output", "[fast][neuralnetwork][sequence]") {
    std::vector<Image::pointer> images;

    // Import data
    {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
        auto port = importer->getOutputPort();
        importer->update(0);
        auto data = port->getNextFrame<Image>();
        images.push_back(data);
        images.push_back(data);
        images.push_back(data);
    }
    auto sequence = Sequence::New();
    sequence->create(images);

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/temporal_input_static_output.pb");
    network->addOutputNode(0, "dense/BiasAdd", NeuralNetwork::NodeType::TENSOR);
    network->setInputData(sequence);
    auto port = network->getOutputPort(0);
    network->update(0);
    auto data = port->getNextFrame<Tensor>();

    REQUIRE(data->getShape().getDimensions() == 2);
    CHECK(data->getShape()[0] == 1);
    CHECK(data->getShape()[1] == 10);
}

TEST_CASE("NN: temporal input temporal output", "[fast][neuralnetwork][sequence]") {
    std::vector<Image::pointer> images;

    // Import data
    {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
        auto port = importer->getOutputPort();
        importer->update(0);
        auto data = port->getNextFrame<Image>();
        images.push_back(data);
        images.push_back(data);
        images.push_back(data);
    }
    auto sequence = Sequence::New();
    sequence->create(images);

    auto network = NeuralNetwork::New();
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/temporal_input_temporal_output.pb");
    network->addOutputNode(0, "lstm/transpose_1", NeuralNetwork::NodeType::TENSOR);
    network->setInputData(sequence);
    auto port = network->getOutputPort(0);
    network->update(0);
    auto data = port->getNextFrame<Tensor>();

    REQUIRE(data->getShape().getDimensions() == 3);
    CHECK(data->getShape()[0] == 1);
    CHECK(data->getShape()[1] == 3); // Timesteps
    CHECK(data->getShape()[2] == 10);
}