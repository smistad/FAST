#include <FAST/Testing.hpp>
#include "NeuralNetwork.hpp"
#include "SegmentationNetwork.hpp"
#include "InferenceEngineManager.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/ImageCropper/ImageCropper.hpp>
#include <FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp>
#include <FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp>
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget/PlaybackWidget.hpp>

using namespace fast;

TEST_CASE("Execute NN on single 2D image", "[fast][neuralnetwork][visual][ultrasound]") {
    //Reporter::setGlobalReportMethod(Reporter::NONE);
    //Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
        if(engine == "OpenVINO") {
            // On OpenVINO, try all device types
            deviceTypes = std::map<std::string, InferenceDeviceType>{
                    {"CPU", InferenceDeviceType::CPU},
                    {"GPU", InferenceDeviceType::GPU},
                    {"VPU", InferenceDeviceType::VPU},
            };
        }
        for(auto&& deviceType : deviceTypes) {
            std::cout << engine << " for device type " << deviceType.first << std::endl;
            std::cout << "====================================" << std::endl;
            //auto importer = ImageFileImporter::New();
            //importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
            auto streamer = ImageFileStreamer::New();
            streamer->setFilenameFormat(Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd");
            streamer->enableLooping();

            auto segmentation = SegmentationNetwork::New();
            segmentation->setInferenceEngine(engine);
            segmentation->getInferenceEngine()->setDeviceType(deviceType.second);
            try {
                segmentation->load(join(Config::getTestDataPath(),
                                        "NeuralNetworkModels/jugular_vein_segmentation." +
							  getModelFileExtension(segmentation->getInferenceEngine()->getPreferredModelFormat())));
            } catch(Exception &e) {
                // If a device type is not present, it will fail in load
                if(engine == "OpenVINO") {
                    Reporter::warning() << e.what() << Reporter::end();
                    continue;
                } else {
                    throw e;
                }
            }
            segmentation->setScaleFactor(1.0f / 255.0f);
            //segmentation->setInputConnection(importer->getOutputPort());
            segmentation->setInputConnection(streamer->getOutputPort());
            segmentation->enableRuntimeMeasurements();

            auto segmentationRenderer = SegmentationRenderer::New();
            segmentationRenderer->addInputConnection(segmentation->getOutputPort());
            segmentationRenderer->setOpacity(0.25);
            segmentationRenderer->setColor(1, Color::Red());
            segmentationRenderer->setColor(2, Color::Blue());

            auto imageRenderer = ImageRenderer::New();
            //imageRenderer->setInputConnection(importer->getOutputPort());
            imageRenderer->setInputConnection(streamer->getOutputPort());

            auto window = SimpleWindow::New();
            window->addRenderer(imageRenderer);
            window->addRenderer(segmentationRenderer);
            window->set2DMode();
            window->setTimeout(5000);
            window->start();
            segmentation->getAllRuntimes()->printAll();
        }
    }
}

TEST_CASE("Execute NN on single 3D image", "[fast][neuralnetwork][3d][visual]") {
    /*
    auto importer = ImageFileImporter::New();
    //importer->setFilename("/home/smistad/3000611.mhd");
    importer->setFilename("/home/smistad/3000611-03264/000001.dcm");
    importer->setMainDevice(DeviceManager::getInstance()->getDefaultDevice());

    auto cropper = ImageCropper::New();
    cropper->setInputConnection(importer->getOutputPort());
    cropper->setOffset(Vector3i(0, 0, 32));
    cropper->setSize(Vector3i(512, 512, 64));

    auto segmentation = SegmentationNetwork::New();
    //segmentation->setHeatmapOutput();
    segmentation->setNrOfClasses(2);
    segmentation->load(Config::getTestDataPath() + "NeuralNetworkModels/lung_nodule_segmentation.pb");
    segmentation->setOutputNode(0, "conv3d_19/truediv");
    segmentation->setInputConnection(cropper->getOutputPort());
    segmentation->enableRuntimeMeasurements();

    auto sliceRenderer = SliceRenderer::New();
    sliceRenderer->addInputConnection(cropper->getOutputPort());
    sliceRenderer->setOrthogonalSlicePlane(0, PLANE_Z);
    sliceRenderer->setIntensityLevel(-512);
    sliceRenderer->setIntensityWindow(1024);

    //auto smoothing = GaussianSmoothing::New();
    //smoothing->setInputConnection(segmentation->getOutputPort());
    //smoothing->setOutputType(TYPE_FLOAT);
    //smoothing->setStandardDeviation(2.0);

    auto surfaceExtraction = SurfaceExtraction::create();
    surfaceExtraction->setInputConnection(segmentation->getOutputPort(0));
    surfaceExtraction->setThreshold(0.1);

    auto triangleRenderer = TriangleRenderer::New();
    triangleRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(sliceRenderer);
    window->addRenderer(triangleRenderer);
    window->set3DMode();
    //window->setTimeout(1000);
    window->start();

    segmentation->getAllRuntimes()->printAll();
    */
}

TEST_CASE("Multi input single output network", "[fast][neuralnetwork]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        network->load(join(Config::getTestDataPath(),
                                "NeuralNetworkModels/multi_input_single_output." +
                                getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputConnection(0, importer->getOutputPort());
        network->setInputConnection(1, importer->getOutputPort());
        auto port = network->getOutputPort();
        network->update();
        auto data = port->getNextFrame<Tensor>();
        // We are expecting a tensor output with dimensions (6)
        REQUIRE(data->getShape().getDimensions() == 1);
        CHECK(data->getShape()[0] == 6);
    }
}

TEST_CASE("Multi input single output network with batch", "[fast][neuralnetwork]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
        auto image = importer->runAndGetOutputData<Image>();
        auto batch1 = Batch::create({image, image});
        auto batch2 = Batch::create({image, image});

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/multi_input_single_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->connect(0, batch1);
        network->connect(1, batch2);
        auto batch = network->runAndGetOutputData<Batch>();
        auto list = batch->get().getTensors();
        REQUIRE(list.size() == 2);
        auto data = list[0];
        // We are expecting a tensor output with dimensions (6)
        REQUIRE(data->getShape().getDimensions() == 1);
        CHECK(data->getShape()[0] == 6);
    }
}

TEST_CASE("Single input multi output network", "[fast][neuralnetwork]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
#ifdef WIN32
#elif defined(__APPLE__) || defined(__MACOSX)
#else
        if(engine == "ONNXRuntime") {
            // This test hangs on linux for ONNXRuntime for some reason
            continue;
        }
#endif
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        if(engine == "TensorFlow") {
            // TensorFlow needs to know about the two nodes
            network->setOutputNode(0, "dense_1/BiasAdd", NodeType::TENSOR);
            network->setOutputNode(1, "dense_2/BiasAdd", NodeType::TENSOR);
        }
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/single_input_multi_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputConnection(0, importer->getOutputPort());
        auto port1 = network->getOutputPort(0);
        auto port2 = network->getOutputPort(1);
        network->update();
        auto data1 = port1->getNextFrame<Tensor>();
        auto data2 = port2->getNextFrame<Tensor>();

        // We are expecting two tensors as output each with dimensions (6)
        REQUIRE(data1->getShape().getDimensions() == 1);
        CHECK(data1->getShape()[0] == 6);
        REQUIRE(data2->getShape().getDimensions() == 1);
        CHECK(data2->getShape()[0] == 6);
    }
}

TEST_CASE("Single 3D image input network", "[fast][neuralnetwork][3d]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        auto importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/single_volume_input." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputConnection(0, importer->getOutputPort());
        auto port = network->getOutputPort();
        network->update();
        auto data = port->getNextFrame<Tensor>();

        // We are expecting one tensor as output with shape (10)
        REQUIRE(data->getShape().getDimensions() == 1);
        CHECK(data->getShape()[0] == 10);
    }
}

TEST_CASE("Execute NN on batch of 2D images", "[fast][neuralnetwork][batch]") {
    for(auto&& engine : InferenceEngineManager::getEngineList()) {
        std::cout << engine << " for device type " << std::endl;
        std::cout << "====================================" << std::endl;

#ifdef WIN32
#elif defined(__APPLE__) || defined(__MACOSX)
#else
        if(engine == "ONNXRuntime") {
            // This test hangs on linux for ONNXRuntime for some reason
            continue;
        }
#endif

        std::vector<Image::pointer> images;

        // Import data
        {
            auto importer = ImageFileImporter::New();
            importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
            auto port = importer->getOutputPort();
            importer->update();
            auto data = port->getNextFrame<Image>();
            images.push_back(data);
            images.push_back(data);
        }
        auto batch = Batch::create(images);

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        network->getInferenceEngine()->setMaxBatchSize(2);
        if(engine == "TensorFlow") {
            // TensorFlow needs to know about the two nodes
            network->setOutputNode(0, "dense_1/BiasAdd", NodeType::TENSOR);
            network->setOutputNode(1, "dense_2/BiasAdd", NodeType::TENSOR);
        }
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/single_input_multi_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputData(batch);
        auto port1 = network->getOutputPort(0);
        auto port2 = network->getOutputPort(1);
        network->update();
        auto data1 = port1->getNextFrame<Batch>();
        auto data2 = port2->getNextFrame<Batch>();

        // We are expecting two batches as output each with dimensions (6)
        auto list1 = data1->get();

        REQUIRE(list1.getSize() == 2);
        CHECK(list1.isTensors());
        CHECK_FALSE(list1.isImages());

        CHECK(list1.getTensors()[0]->getShape()[0] == 6);
        CHECK(list1.getTensors()[1]->getShape()[0] == 6);

        auto list2 = data2->get();

        REQUIRE(list2.getSize() == 2);
        CHECK(list2.isTensors());
        CHECK_FALSE(list2.isImages());

        CHECK(list2.getTensors()[0]->getShape()[0] == 6);
        CHECK(list2.getTensors()[1]->getShape()[0] == 6);
    }
}

TEST_CASE("NN: temporal input static output", "[fast][neuralnetwork][sequence]") {
    for(auto&& engine : InferenceEngineManager::getEngineList()) {
        std::vector<Image::pointer> images;

        // Import data
        {
            auto importer = ImageFileImporter::New();
            importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
            auto port = importer->getOutputPort();
            importer->update();
            auto data = port->getNextFrame<Image>();
            images.push_back(data);
            images.push_back(data);
            images.push_back(data);
        }
        auto sequence = Sequence::create(images);

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::OPENVINO) // OPENVINO not able to convert this model yet
            continue;
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::ONNX) // ONNX not able to convert this model yet
            continue;
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/temporal_input_static_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputData(sequence);
        auto port = network->getOutputPort(0);
        network->update();
        auto data = port->getNextFrame<Tensor>();

        REQUIRE(data->getShape().getDimensions() == 1);
        CHECK(data->getShape()[0] == 10);
    }
}

TEST_CASE("NN: temporal input temporal output", "[fast][neuralnetwork][sequence]") {
    for(auto&& engine : InferenceEngineManager::getEngineList()) {
        std::vector<Image::pointer> images;

        // Import data
        {
            auto importer = ImageFileImporter::New();
            importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
            auto port = importer->getOutputPort();
            importer->update();
            auto data = port->getNextFrame<Image>();
            images.push_back(data);
            images.push_back(data);
            images.push_back(data);
        }
        auto sequence = Sequence::create(images);

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::OPENVINO) // OPENVINO not able to convert this model yet
            continue;
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::ONNX) // ONNX not able to convert this model yet
            continue;
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/temporal_input_temporal_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        network->setInputData(sequence);
        auto port = network->getOutputPort(0);
        network->update();
        auto data = port->getNextFrame<Tensor>();

        REQUIRE(data->getShape().getDimensions() == 2);
        CHECK(data->getShape()[0] == 3); // Timesteps
        CHECK(data->getShape()[1] == 10);
    }
}

TEST_CASE("NN: temporal input temporal output, streaming mode", "[fast][neuralnetwork][sequence]") {
    for(auto&& engine : InferenceEngineManager::getEngineList()) {
        std::vector<Image::pointer> images;

        // Import data
        {
            auto importer = ImageFileImporter::New();
            importer->setFilename(Config::getTestDataPath() + "US/JugularVein/US-2D_0.mhd");
            auto port = importer->getOutputPort();
            importer->update();
            auto data = port->getNextFrame<Image>();
            images.push_back(data);
            images.push_back(data);
            images.push_back(data);
        }

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        network->setTemporalWindow(3);
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::OPENVINO) // OPENVINO not able to convert this model yet
            continue;
        if(network->getInferenceEngine()->getPreferredModelFormat() == ModelFormat::ONNX) // ONNX not able to convert this model yet
            continue;
        network->load(join(Config::getTestDataPath(),
                           "NeuralNetworkModels/temporal_input_temporal_output." +
                           getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())));
        auto port = network->getOutputPort(0);

        for(int i = 0; i < 3; ++i) {
            network->setInputData(images[i]);
            network->update();
            auto data = port->getNextFrame<Tensor>();

            REQUIRE(data->getShape().getDimensions() == 2);
            CHECK(data->getShape()[0] == 3); // Timesteps
            CHECK(data->getShape()[1] == 10);
        }
    }
}

/*
TEST_CASE("Dynamic input shapes", "[fast][dynamicshapes]") {
    auto network = NeuralNetwork::create("/home/smistad/workspace/adapt-ai-tuning/models/unet-adapt-rspace-full-res-ssim-1.5-dynamic.onnx",
                                         {NeuralNetworkNode("input_1", NodeType::IMAGE, TensorShape(), 0, TensorShape({1, 20, 20, 1}), TensorShape({1, 1024, 1024, 1}), TensorShape({1, 256, 256, 1}))}, {}, "OpenVINO");
    auto tensor1 = Tensor::create(TensorShape({1, 256, 200, 1}));
    network->setInputData(tensor1);
    network->run();
    auto tensor2 = Tensor::create(TensorShape({1, 256, 200, 1}));
    network->setInputData(tensor2);
    network->run();
    auto tensor3 = Tensor::create(TensorShape({1, 256, 400, 1}));
    network->setInputData(tensor3);
    network->run();
}

#include "VertexTensorToSegmentation.hpp"
#include <FAST/Algorithms/Lambda/RunLambda.hpp>
#include <FAST/Visualization/LineRenderer/LineRenderer.hpp>

TEST_CASE("VertexTensorToSegmentation", "[fast][VertexTensorToSegmentation]") {

    auto streamer = ImageFileStreamer::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd", true, false, 20);

    auto gray2color = RunLambda::create([](DataObject::pointer input) -> DataList {
       auto inputImage = std::dynamic_pointer_cast<Image>(input);
       auto inputAccess = inputImage->getImageAccess(ACCESS_READ);
       auto output = Image::create(inputImage->getSize(), TYPE_FLOAT, 3);
       output->setSpacing(inputImage->getSpacing());
       auto outputAccess = output->getImageAccess(ACCESS_READ_WRITE);
       for(int i = 0; i < inputImage->getNrOfVoxels(); ++i) {
           float value = (float)inputAccess->getScalarFast<uchar>(i) / 256.0f;
           outputAccess->setScalarFast<float>(i, value, 0);
           outputAccess->setScalarFast<float>(i, value, 1);
           outputAccess->setScalarFast<float>(i, value, 2);
       }
       return DataList(output);
    })->connect(streamer);

    auto network = NeuralNetwork::create("/home/smistad/Downloads/echoGraphFineTuned.onnx", 1.0f, 0.5f, 0.5f)
            ->connect(gray2color);

    Connections connections = {{}, {}, {}};

    int N = 41;
    int N2 = 22;
    int N3 = 41;
    // LV
    for(int i = 0; i < N; ++i) {
        connections[1].push_back({i, i+1});
    }
    connections[1].push_back({0, 63}); // Close it
    connections[1].push_back({N, 63}); // Close it
    // LA
    for(int i = N; i < N+N2; ++i) {
        connections[2].push_back({i, i+1});
    }
    connections[2].push_back({N+N2, N}); // Close it
    // Myocardium
    for(int i = N+N2; i < N+N2+N3; ++i) {
        connections[0].push_back({i, i+1});
    }
    connections[0].push_back({N+N2+N3, N}); // Close it
    connections[0].push_back({N+N2, N}); // Close it

    auto tensorToSeg = VertexTensorToSegmentation::create(connections)
            ->connect(network);
    tensorToSeg->enableRuntimeMeasurements();

    auto imgRenderer = ImageRenderer::create()->connect(gray2color);
    auto segRenderer = SegmentationRenderer::create(LabelColors(), 0.25)->connect(tensorToSeg);
    auto lineRenderer = LineRenderer::create(Color::Green(), 0.5)->connect(tensorToSeg, 1);

    auto widget = new PlaybackWidget(streamer);

    auto window = SimpleWindow2D::create()->connect(imgRenderer)->connect(segRenderer)->connect(lineRenderer);
    window->addWidget(widget);
    window->run();
    tensorToSeg->getRuntime()->print();
}
 */
