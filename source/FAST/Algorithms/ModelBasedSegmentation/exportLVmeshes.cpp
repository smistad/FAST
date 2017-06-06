#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "KalmanFilter.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "AppearanceModels/RidgeEdge/RidgeEdgeModel.hpp"
#include "ShapeModels/CardinalSpline/CardinalSplineModel.hpp"
#include "FAST/Exporters/VTKMeshFileExporter.hpp"
#include "FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp"
#include <chrono>
#include "FAST/Exporters/StreamExporter.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include <fstream>
#include <FAST/Importers/ImageFileImporter.hpp>

using namespace fast;

std::vector<std::string> getPaths(std::string filename) {
    std::vector<std::string> paths;
    std::ifstream file(filename);
    if(!file.is_open()) {
        throw Exception("Unable to open dataset list file");
    }

    std::string line = "";
    std::getline(file, line);
    while(!file.eof()) {
        if(line[0] != '#' && line.size() > 3) {
            if(line.find("#") != std::string::npos) {
                // Remove comment
                line = line.substr(0, line.find("#") - 1);
            }
            paths.push_back(line + "/");
        }
        std::getline(file, line);
    }

    std::cout << "Found " << paths.size() << " paths in file, processing.." << std::endl;

    return paths;
}

int main(int argc, char** argv) {
    bool visualize = false;
    bool prerun = true;
    if(argc > 1) {
        if(std::string(argv[1]) == "--help") {
            std::cout << "Usage: " << argv[0] << " [--visualize] [--no-prerun]" << std::endl;
            return 0;
        }
        for(int i = 1; i < argc; i++) {
            std::string token = argv[i];
            if(token == "--visualize")
                visualize = true;
            if(token == "--no-prerun")
                prerun = false;
        }
    }

    //Reporter::setGlobalReportMethod(Reporter::COUT);
    std::vector<std::string> paths = getPaths("/media/extra/GRUE_MHD/left_ventricle_segmentation_list.txt");

    for(std::string path : paths) {
        // Control points for spline model
        std::vector<Vector2f> controlPoints = {
                Vector2f(89, 22), // Apex
                Vector2f(100, 31),
                Vector2f(108, 50),
                Vector2f(114, 63),
                Vector2f(117, 80),
                Vector2f(113, 95),
                Vector2f(96, 102),
                Vector2f(75, 99),
                Vector2f(73, 81),
                Vector2f(75, 61),
                Vector2f(72, 43),
                Vector2f(80, 31),
        };

        CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
        shapeModel->setControlPoints(controlPoints);
        // Increasing these will put more weight on the measurements instead of the model, and vica versa
        shapeModel->setGlobalProcessError(0.00001f);
        shapeModel->setLocalProcessError(0.001f);
        shapeModel->initializeShapeToImageCenter();
        KalmanFilter::pointer segmentation = KalmanFilter::New();
        /*
        RidgeEdgeModel::pointer appearanceModel = RidgeEdgeModel::New();
        appearanceModel->setEdgeType(RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
        appearanceModel->setIntensityDifferenceThreshold(20);
        appearanceModel->setMinimumRidgeSize(3.0f);
         */
        StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
        appearanceModel->setEdgeType(StepEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
        appearanceModel->setIntensityDifferenceThreshold(15);

        appearanceModel->setLineLength(30.0);
        appearanceModel->setLineSampleSpacing(30.0 / 48.0);
        segmentation->setAppearanceModel(appearanceModel);
        segmentation->setShapeModel(shapeModel);
        segmentation->setIterations(10);
        segmentation->setStartIterations(20);

        // Get first frame to get size of image
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        importer->setFilename(path + "US-2D_0.mhd");
        importer->update();
        Image::pointer image = importer->getOutputData<Image>();

        // Run through recording once first,
        if(prerun) {
            ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
            streamer->setFilenameFormat(path + "US-2D_#.mhd");
            streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
            streamer->setSleepTime(100);
            segmentation->setInputConnection(streamer->getOutputPort());
            streamer->update();
            DynamicData::pointer dynamicMeshData = segmentation->getOutputData<Mesh>();
            int i = 0;
            while (!dynamicMeshData->getStreamer()->hasReachedEnd() && i < 10) {
                segmentation->update();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                i++;
            }
            std::cout << "Finished pre run" << std::endl;
        }
        ImageFileStreamer::pointer streamer2 = ImageFileStreamer::New();
        streamer2->setFilenameFormat(path + "US-2D_#.mhd");
        streamer2->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        streamer2->setSleepTime(100);
        segmentation->setInputConnection(streamer2->getOutputPort());

        if(visualize) {
            streamer2->enableLooping();
            TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
            TriangleRenderer->addInputConnection(segmentation->getOutputPort());
            //TriangleRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

            ImageRenderer::pointer imageRenderer = ImageRenderer::New();
            imageRenderer->addInputConnection(streamer2->getOutputPort());

            SimpleWindow::pointer window = SimpleWindow::New();
            window->getView()->setBackgroundColor(Color::Black());
            window->addRenderer(imageRenderer);
            window->addRenderer(TriangleRenderer);
            window->setSize(1024, 1024);
            window->set2DMode();
            window->setTimeout(10000);
            window->setTitle(path);
            window->start();
        } else {
//            // Export
//            streamer2->update();
//            StreamExporter::pointer streamExporter = StreamExporter::New();
//            streamExporter->setFilenameFormat(path + "segmentation_mesh_#.vtk");
//            VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
//            streamExporter->setExporter(exporter);
//            streamExporter->setInputConnection(segmentation->getOutputPort());
//            std::cout << "Starting " << path << std::endl;
//            while(!streamExporter->isFinished()) {
//                std::cout << "Calling update... " << path << std::endl;
//                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//                streamExporter->update();
//            }
            streamer2->update();
            DynamicData::pointer dynamicMeshData = segmentation->getOutputData<Mesh>();
            int i = 0;
            Mesh::pointer previous;
            VTKMeshFileExporter::pointer dummyExporter = VTKMeshFileExporter::New();
            while(!dynamicMeshData->hasReachedEnd()) {
                segmentation->update();
                try {
                    Mesh::pointer mesh = dynamicMeshData->getNextFrame(dummyExporter);
                    if(mesh == previous)
                        continue;
                    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
                    meshToSeg->setInputData(0, mesh);
                    meshToSeg->setInputData(1, image);

                    MetaImageExporter::pointer exporter = MetaImageExporter::New();
                    exporter->setInputConnection(meshToSeg->getOutputPort());
                    exporter->setFilename(path + "segmentation_" + std::to_string(i) + ".mhd");
                    exporter->update();

                    previous = mesh;
                    i++;
                } catch(Exception &e) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
            }
            std::cout << "Finished " << path << std::endl;
            std::cout << "---------------------------" << std::endl;
        }
    }
    std::cout << "Finished processing all." << std::endl;
}
