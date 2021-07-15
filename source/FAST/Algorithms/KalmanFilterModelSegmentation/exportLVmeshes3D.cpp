#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "KalmanFilterModelSegmentation.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "AppearanceModels/RidgeEdge/RidgeEdgeModel.hpp"
#include "ShapeModels/MeanValueCoordinates/MeanValueCoordinatesModel.hpp"
#include "FAST/Exporters/VTKMeshFileExporter.hpp"
#include "FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp"
#include <chrono>
#include "FAST/Exporters/StreamExporter.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include <fstream>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

// CETUS model is in meters, must convert from mm to m
class ConvertToMeters : public ProcessObject {
    FAST_OBJECT(ConvertToMeters)
public:
private:
    ConvertToMeters() {
        createInputPort<Image>(0);
        createOutputPort<Image>(0);
    }
    void execute() {
        auto input = getInputData<Image>(0);
        input->setSpacing(input->getSpacing() / 1000.0f);
        addOutputData(0, input);
    }
};


int main(int argc, char** argv) {

    CommandLineParser parser("Left ventricle segmentation in 3D using Kalman filter model based approach");
    parser.addPositionVariable(1, "filepath", true);
    parser.addOption("visualize", "Visualize?");

    parser.parse(argc, argv);

    const bool visualize = parser.getOption("visualize");



    for(auto path : getDirectoryList(parser.get("filepath"))) {
        if(path == "segmentation")
            continue;
        std::cout << "Processing " << path << std::endl;
        const std::string storagePath = parser.get("filepath") + "/segmentation/" + path + "/";
        if(fileExists(storagePath))
            continue;
        createDirectories(storagePath);
        path = parser.get("filepath") + "/" + path + "/";

        auto streamer = ImageFileStreamer::New();
        streamer->setFilenameFormat(path + "frame_#.mhd");
        streamer->enableLooping();

        auto convert = ConvertToMeters::New();
        convert->setInputConnection(streamer->getOutputPort());

        auto shapeModel = MeanValueCoordinatesModel::New();
        shapeModel->loadMeshes(Config::getTestDataPath() + "cetus_model_mesh_small.vtk",
            Config::getTestDataPath() + "cetus_control_mesh.vtk");
        shapeModel->initializeShapeToImageCenter();
        shapeModel->setInitialScaling(0.7, 0.7, 0.7);
        //shapeModel->setInitialScaling(1000, 1000, 1000); // Cetus model is in meters, but our images are in millimeters.
        auto segmentation = KalmanFilter::New();
        auto appearanceModel = StepEdgeModel::New();
        appearanceModel->setLineLength(0.025);
        appearanceModel->setLineSampleSpacing(0.025 / 48.0);
        appearanceModel->setEdgeType(StepEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
        appearanceModel->setIntensityDifferenceThreshold(10);
        segmentation->setAppearanceModel(appearanceModel);
        segmentation->setShapeModel(shapeModel);
        //segmentation->setIterations(1);
        //segmentation->setStartIterations(1);
        segmentation->setInputConnection(convert->getOutputPort());

        if(!visualize) {
            auto meshToSeg = MeshToSegmentation::New();
            meshToSeg->setInputConnection(0, segmentation->getOutputPort());
            meshToSeg->setInputConnection(1, convert->getOutputPort());

            auto port = meshToSeg->getOutputPort();
            int frames = streamer->getNrOfFrames();

            // Run through one time first
            for(int i = 0; i < frames; ++i) {
                std::cout << "processing frame " << i << std::endl;
                meshToSeg->update();
                port->getNextFrame<Segmentation>();
            }
            for(int i = 0; i < frames; ++i) {
                std::cout << "processing frame " << i << std::endl;
                meshToSeg->update();
                auto segData = port->getNextFrame<Segmentation>();
                // Export
                auto exporter = MetaImageExporter::New();
                exporter->enableCompression();
                exporter->setFilename(storagePath + "/frame_" + std::to_string(i) + ".mhd");
                exporter->setInputData(segData);
                exporter->update();
            }
        } else {

            auto TriangleRenderer = TriangleRenderer::New();
            TriangleRenderer->setInputConnection(segmentation->getOutputPort());
            TriangleRenderer->setDefaultOpacity(0.5);

            auto sliceRenderer = SliceRenderer::New();
            sliceRenderer->addInputConnection(convert->getOutputPort(), PLANE_X);
            sliceRenderer->addInputConnection(convert->getOutputPort(), PLANE_Y);

            auto window = SimpleWindow::New();
            window->addRenderer(sliceRenderer);
            window->addRenderer(TriangleRenderer);
            window->start();
        }
    }
    std::cout << "Finished processing all." << std::endl;
}
