#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/LaplacianOfGaussian/LaplacianOfGaussian.hpp>
#include "ApplyColormap.hpp"

using namespace fast;

TEST_CASE("ApplyColormap", "[fast][ApplyColormap][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");
    auto colormap = Colormap({
        {0, 0},
        {100, 90},
        {200, 200},
        {255, 255},
        }, true);
    auto applyColormap = ApplyColormap::create(colormap)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(applyColormap);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}


TEST_CASE("ApplyColormap CoolWarm", "[fast][ApplyColormap][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    auto log = LaplacianOfGaussian::create()->connect(importer);

    auto applyColormap = ApplyColormap::create(Colormap::CoolWarm())->connect(log);

    auto renderer = ImageRenderer::create()->connect(applyColormap);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}
