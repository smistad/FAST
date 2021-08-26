#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"

using namespace fast;

TEST_CASE("DualViewWindow with horizontal mode", "[fast][DualViewWindow][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/US-2D.jpg");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"/US/Ball/US-3Dt_#.mhd");

    SurfaceExtraction::pointer extractor = SurfaceExtraction::create();
    extractor->setInputConnection(mhdStreamer->getOutputPort());
    extractor->setThreshold(200);

    TriangleRenderer::pointer renderer2 = TriangleRenderer::New();
    renderer2->addInputConnection(extractor->getOutputPort());

    auto window = DualViewWindow::create();
    window->addRendererToBottomRightView(renderer);
    window->addRendererToTopLeftView(renderer2);
    window->setTimeout(2000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("DualViewWindow with vertical mode", "[fast][DualViewWindow][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/US-2D.jpg");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"/US/Ball/US-3Dt_#.mhd");

    SurfaceExtraction::pointer extractor = SurfaceExtraction::create();
    extractor->setInputConnection(mhdStreamer->getOutputPort());
    extractor->setThreshold(200);

    TriangleRenderer::pointer renderer2 = TriangleRenderer::New();
    renderer2->addInputConnection(extractor->getOutputPort());

    auto window = DualViewWindow::create()
            ->connectRight(renderer)
            ->connectLeft(renderer2);
    window->setVerticalMode();
    window->setTimeout(2000);
    CHECK_NOTHROW(window->run());
}
