#include "catch.hpp"
#include "ImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageRenderer with single 2D image", "[fast][ImageRenderer]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"lena.jpg");
    CHECK_NOTHROW(
        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->setInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(500);
        window->runMainLoop();
    );
}
