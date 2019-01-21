#include <FAST/Testing.hpp>
#include "MaximumIntensityProjection.hpp"
#include "ThresholdVolumeRenderer.hpp"
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>

using namespace fast;

TEST_CASE("Maximum intensity projection", "[fast][volumerenderer][visual]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = MaximumIntensityProjection::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->start();
}


TEST_CASE("Threshold volume renderer", "[fast][volumerenderer][visual][thresholdvolumerenderer]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = ThresholdVolumeRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    renderer->setThreshold(500);

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->start();
}

TEST_CASE("Volume renderer with geom", "[fast][volumerenderer][visual][asdasdasd]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = MaximumIntensityProjection::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto renderer2 = SliceRenderer::New();
    renderer2->addInputConnection(importer->getOutputPort());
    renderer2->setArbitrarySlicePlane(0, Plane(Vector3f(0.5, 0.5, 0.5)));

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(renderer2);
    window->setTimeout(1000);
    window->start();
}
