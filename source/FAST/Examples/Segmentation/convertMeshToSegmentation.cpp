#include <FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp>
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include <FAST/Importers/MetaImageImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp>
#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>

using namespace fast;

int main(int argc, char** argv) {

    // TODO add support for 2D

    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Convert mesh to segmentation (3D only)");
    parser.addPositionVariable(1, "mesh-filename", Config::getTestDataPath() + "/Surface_LV.vtk");
    parser.addVariable("segmentation-size", false, "Size of segmentation. Example: 256,256,256");
    parser.addVariable("image-filename", Config::getTestDataPath() + "/US/Ball/US-3Dt_0.mhd");
    parser.addVariable("output-filename", false, "Filename to store the segmentation in. Example: /path/to/file.mhd");

    parser.parse(argc, argv);

    auto importer = VTKMeshFileImporter::New();
    importer->setFilename(parser.get("mesh-filename"));

    auto converter = MeshToSegmentation::New();
    converter->setInputConnection(importer->getOutputPort());

    if(parser.gotValue("segmentation-size")) {
        auto size = split(parser.get("segmentation-size"), ",");
        if (size.size() != 3)
            throw Exception("Unable to format input argument segmentation-size: " + parser.get("segmentation-size") +
                            " expected: X,Y,Z");
        try {
            converter->setOutputImageResolution(std::stoi(size[0]), std::stoi(size[1]), std::stoi(size[2]));
        } catch (std::exception &e) {
            throw Exception("Unable to format input argument segmentation-size: " + parser.get("segmentation-size") +
                            " expected: X,Y,Z");
        }
    } else if(parser.gotValue("image-filename")) {
        auto importer2 = MetaImageImporter::New();
        importer2->setFilename(parser.get("image-filename"));
        converter->setInputConnection(1, importer2->getOutputPort());
    } else {
        throw Exception("Need to supply program with either segmentation-size or image-filename");
    }

    if(parser.gotValue("output-filename")) {
        auto exporter = MetaImageExporter::New();
        exporter->setFilename(parser.get("output-filename"));
        exporter->enableCompression();
        exporter->setInputConnection(converter->getOutputPort());
        exporter->update();
    } else {
        // Visualize
        auto triangleRenderer = TriangleRenderer::New();
        triangleRenderer->addInputConnection(importer->getOutputPort());
        triangleRenderer->setOpacity(0, 0.25);

        auto sliceRenderer = SliceRenderer::New();
        sliceRenderer->addInputConnection(converter->getOutputPort());
        sliceRenderer->setOrthogonalSlicePlane(0, PLANE_Z);
        sliceRenderer->setIntensityWindow(1);
        sliceRenderer->setIntensityLevel(0.5);

        auto window = SimpleWindow::New();
        window->addRenderer(triangleRenderer);
        window->addRenderer(sliceRenderer);
        window->start();
    }

}
