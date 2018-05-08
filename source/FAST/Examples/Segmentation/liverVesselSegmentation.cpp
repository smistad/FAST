#include <FAST/Algorithms/HounsefieldConverter/HounsefieldConverter.hpp>
#include "FAST/Algorithms/TubeSegmentationAndCenterlineExtraction/TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    // NOTE: This code is atm made for signed 16 bit integer CT data
    if(argc < 2) {
        std::cout << "usage: " << argv[0] << " /path/to/CT_file" << std::endl;
        return 0;
    }
    std::string filename = argv[1];
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(filename);

    HounsefieldConverter::pointer converter = HounsefieldConverter::New();
    converter->setInputConnection(importer->getOutputPort());

    TubeSegmentationAndCenterlineExtraction::pointer tubeExtraction = TubeSegmentationAndCenterlineExtraction::New();
    tubeExtraction->setInputConnection(converter->getOutputPort());

    // Parameters
    tubeExtraction->extractBrightTubes();
    tubeExtraction->setMinimumRadius(1);
    tubeExtraction->setMaximumRadius(25);
    tubeExtraction->setRadiusStep(0.5);
    tubeExtraction->setSensitivity(0.95);
    tubeExtraction->setMinimumIntensity(100);
    tubeExtraction->setMaximumIntensity(200);
    tubeExtraction->setMinimumTreeSize(500);
    tubeExtraction->setKeepLargestTree(true);
    tubeExtraction->enableAutomaticCropping();

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->addInputConnection(importer->getOutputPort(), PLANE_Z);

    LineRenderer::pointer lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(tubeExtraction->getCenterlineOutputPort(), Color::Blue(), 1);
    lineRenderer->setDefaultDrawOnTop(true);

    SurfaceExtraction::pointer surfaceExtraction = SurfaceExtraction::New();
    surfaceExtraction->setInputConnection(tubeExtraction->getSegmentationOutputPort());

    TriangleRenderer::pointer triangleRenderer = TriangleRenderer::New();
    triangleRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(triangleRenderer);
    window->addRenderer(lineRenderer);
    window->start();
}