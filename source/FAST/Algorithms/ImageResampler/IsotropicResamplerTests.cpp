#include <FAST/Algorithms/ImageSlicer/ImageSlicer.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/ImageResampler/IsotropicResampler.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("IsotropicResampler 2D", "[fast][IsotropicResampler][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    auto resampler = IsotropicResampler::create()->connect(importer);

    auto renderer = ImageRenderer::create()->connect(resampler);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(500);
    window->run();
    auto image = resampler->runAndGetOutputData<Image>();
    CHECK(image->getSpacing().x() == image->getSpacing().y());
}

TEST_CASE("IsotropicResampler 3D CT", "[fast][IsotropicResampler][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    auto resampler = IsotropicResampler::create()->connect(importer);

    ImageSlicer::pointer slicer = ImageSlicer::New();
    slicer->setOrthogonalSlicePlane(PLANE_X);
    slicer->setInputConnection(resampler->getOutputPort());

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(slicer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(500);
    window->start();
    auto image = resampler->runAndGetOutputData<Image>();
    CHECK(image->getSpacing().x() == image->getSpacing().y());
    CHECK(image->getSpacing().y() == image->getSpacing().z());
}