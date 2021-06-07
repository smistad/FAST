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

    auto importer = ImageFileImporter::New();
    importer->setFilename(filename);

    auto converter = HounsefieldConverter::New();
    converter->setInputConnection(importer->getOutputPort());

    auto tubeExtraction = TubeSegmentationAndCenterlineExtraction::New();
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

    auto renderer = SliceRenderer::New();
    renderer->addInputConnection(importer->getOutputPort(), PLANE_Z);

    auto lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(tubeExtraction->getCenterlineOutputPort(), Color::Blue(), 1);
    lineRenderer->setDefaultDrawOnTop(true);

    auto surfaceExtraction = SurfaceExtraction::create();
    surfaceExtraction->setInputConnection(tubeExtraction->getSegmentationOutputPort());

    auto triangleRenderer = TriangleRenderer::New();
    triangleRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(triangleRenderer);
    window->addRenderer(lineRenderer);
    window->start();
}