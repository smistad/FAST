#include <FAST/Testing.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>

using namespace fast;

TEST_CASE("Multi view window", "[fast][MultiViewWindow][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    auto renderer1 = ImageRenderer::create(100, 100)->connect(importer);
    auto renderer2 = ImageRenderer::create(200, 100)->connect(importer);
    auto renderer3 = ImageRenderer::create(50, 200)->connect(importer);

    auto window = MultiViewWindow::create(3)
            ->connect(0, renderer1)
            ->connect(1, renderer2)
            ->connect(2, renderer3);

    window->setTimeout(1000);
    window->run();

}