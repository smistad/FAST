#include <FAST/Testing.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>
#include <fstream>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);

    CommandLineParser parser("Measure neural network performance script");
    parser.addOption("disable-case-1");
    parser.addOption("disable-case-2");
    parser.addOption("disable-case-3");
    parser.addOption("disable-case-3-batch");
    parser.addOption("disable-warmup");
    parser.parse(argc, argv);
    const int iterations = 10;
    const bool warmupIteration = !parser.getOption("disable-warmup");
    const bool case1 = !parser.getOption("disable-case-1");
    const bool case2 = !parser.getOption("disable-case-2");
    const bool case3 = !parser.getOption("disable-case-3");
    const bool case3_batch = !parser.getOption("disable-case-3-batch");

    if(case1) {
        // CASE 1 - ULTRASOUND
        const std::string resultFilename = "neural-network-runtimes-case-1.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Total\n";

        for(auto &engine : InferenceEngineManager::getEngineList()) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        {"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                    auto streamer = ImageFileStreamer::New();
                    streamer->setFilenameFormat(Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd");

                    auto segmentation = SegmentationNetwork::New();
                    segmentation->setInferenceEngine(engine);
                    segmentation->getInferenceEngine()->setDeviceType(deviceType.second);
                    std::string postfix = "";
                    if(engine == "TensorFlow") {
                        segmentation->setOutputNode(0, "conv2d_23/truediv");
                    } else if(engine == "TensorRT") {
                        segmentation->setInputNode(0, "input_image", NodeType::IMAGE, TensorShape({-1, 1, 256, 256}));
                        segmentation->setOutputNode(0, "permute_2/transpose", NodeType::TENSOR,
                                                    TensorShape({-1, 3, 256, 256}));
                    } else if(engine == "OpenVINO") {
                        if (deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }

                    try {
                        segmentation->load(join(Config::getTestDataPath(),
                                                "NeuralNetworkModels/jugular_vein_segmentation"+postfix+"." +
							  getModelFileExtension(segmentation->getInferenceEngine()->getPreferredModelFormat())));
                    } catch(Exception &e) {
                        // If a device type is not present, it will fail in load
                        Reporter::warning() << e.what() << Reporter::end();
                        continue;
                    }
                    segmentation->setScaleFactor(1.0f / 255.0f);
                    //segmentation->setInputConnection(importer->getOutputPort());
                    segmentation->setInputConnection(streamer->getOutputPort());
                    segmentation->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = segmentation->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed =
                            std::chrono::high_resolution_clock::now() - start;
                    std::cout << "RUNTIME Ultrasound " << engine << std::endl;
                    std::cout << "=========================================" << std::endl;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "NN runtime: " << std::endl;
                    segmentation->getRuntime("input_processing")->print();
                    segmentation->getRuntime("inference")->print();
                    segmentation->getRuntime("output_processing")->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(segmentation->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(segmentation->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(segmentation->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(segmentation->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(segmentation->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(segmentation->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }


    if(case2) {
        // CASE 2 - VOLUME SEGMENTATION
        const std::string resultFilename = "neural-network-runtimes-case-2.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //Reporter::setGlobalReportMethod(Reporter::NONE);
        for(std::string engine : {"TensorFlow"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                   auto importer = ImageFileImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/CT/LIDC-IDRI-0072/000001.dcm");

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(512, 512, 32);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = SegmentationNetwork::New();
                    network->setInferenceEngine(engine);
                    network->getInferenceEngine()->setDeviceType(deviceType.second);
                    network->setInferenceEngine(engine);
                    network->load(Config::getTestDataPath() + "/NeuralNetworkModels/lung_nodule_segmentation.pb");
                    network->setMinAndMaxIntensity(-1200.0f, 400.0f);
                    network->setScaleFactor(1.0f/(400+1200));
                    network->setMeanAndStandardDeviation(-1200.0f, 1.0f);
                    network->setOutputNode(0, "conv3d_14/truediv");
                    network->setInputConnection(generator->getOutputPort());
                    network->setResizeBackToOriginalSize(true);
                    network->setThreshold(0.3);
                    network->enableRuntimeMeasurements();

                    auto stitcher = PatchStitcher::New();
                    stitcher->setInputConnection(network->getOutputPort());
                    stitcher->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = stitcher->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    stitcher->getRuntime()->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }


    if(case3) {
        // CASE 3 - WSI CLASSIFICATION
        const std::string resultFilename = "neural-network-runtimes-case-3.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        for(auto &engine : InferenceEngineManager::getEngineList()) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        {"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                    auto importer = WholeSlideImageImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                    auto tissueSegmentation = TissueSegmentation::New();
                    tissueSegmentation->setInputConnection(importer->getOutputPort());

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(512, 512);
                    generator->setPatchLevel(0);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = NeuralNetwork::New();
                    network->setInferenceEngine(engine);
                    std::string postfix;
                    if(engine == "TensorFlow") {
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
                        if(deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }
                    network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." +
							  getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat()));
                    network->setInputConnection(generator->getOutputPort());
                    network->setScaleFactor(1.0f / 255.0f);
                    network->enableRuntimeMeasurements();

                    auto stitcher = PatchStitcher::New();
                    stitcher->setInputConnection(network->getOutputPort());
                    stitcher->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = stitcher->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    stitcher->getRuntime()->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }

    if(case3_batch) {
        // CASE 3 - WSI CLASSIFICATION with batches
        const int batchSize = 16;
        const std::string resultFilename = "neural-network-runtimes-case-3-batch.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        for(std::string engine : {"TensorRT", "TensorFlow", "OpenVINO"}) {
            if(!InferenceEngineManager::isEngineAvailable(engine))
                continue;
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        //{"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        {"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                    auto importer = WholeSlideImageImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                    auto tissueSegmentation = TissueSegmentation::New();
                    tissueSegmentation->setInputConnection(importer->getOutputPort());

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(512, 512);
                    generator->setPatchLevel(0);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto batchGenerator = ImageToBatchGenerator::New();
                    batchGenerator->setMaxBatchSize(16);

                    auto network = NeuralNetwork::New();
                    network->setInferenceEngine(engine);
                    network->getInferenceEngine()->setMaxBatchSize(batchSize);
                    std::string postfix;
                    if(engine == "TensorFlow") {
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
                        postfix = "batch_" + std::to_string(batchSize);
                        if(deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }
                    network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." +
							  getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat()));
                    network->setInputConnection(generator->getOutputPort());
                    network->setScaleFactor(1.0f / 255.0f);
                    network->enableRuntimeMeasurements();

                    auto stitcher = PatchStitcher::New();
                    stitcher->setInputConnection(network->getOutputPort());
                    stitcher->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = stitcher->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    stitcher->getRuntime()->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }
}
