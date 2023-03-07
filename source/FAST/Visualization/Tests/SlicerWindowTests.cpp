#include <FAST/Testing.hpp>
#include <FAST/Visualization/SlicerWindow.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>

using namespace fast;

TEST_CASE("Slicer window", "[fast][SlicerWindow][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto window = SlicerWindow::create()
            ->connectImage(importer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Slicer window with segmentation", "[fast][SlicerWindow][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto threshold = BinaryThresholding::create(150)
            ->connect(importer);

    auto window = SlicerWindow::create()
            ->connectImage(importer, 0, 1024)
            ->connectSegmentation(threshold, {{1, Color::Blue()}}, 0.25, 0.75);
    window->setTimeout(1000);
    window->run();
}


TEST_CASE("Slicer window with only segmentation", "[fast][SlicerWindow][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto threshold = BinaryThresholding::create(100)
            ->connect(importer);

    auto window = SlicerWindow::create()
            ->connectSegmentation(threshold);
    window->setTimeout(1000);
    window->run();
}
TEST_CASE("Slicer window with multiple segmentations", "[fast][SlicerWindow][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto threshold = BinaryThresholding::create(150)
            ->connect(importer);

    auto threshold2 = BinaryThresholding::create(-1024, -700)
            ->connect(importer);

    auto window = SlicerWindow::create()
            ->connectImage(importer, 0, 1024)
            ->connectSegmentation(threshold, {{1, Color::Blue()}}, 0.25, 0.75)
            ->connectSegmentation(threshold2, {{1, Color::Green()}}, 0.25, 0.75);
    window->setTimeout(1000);
    window->run();
}

