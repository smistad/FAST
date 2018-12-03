#include <FAST/Testing.hpp>
#include "TemplateMatchingNCC.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

using namespace fast;

TEST_CASE("Template matching NCC", "[fast][NCC][TemplateMatching]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(join(Config::getTestDataPath(), "US/CarotidArtery/Left/US-2D_300.mhd"));
    importer->setMainDevice(DeviceManager::getInstance()->getOneGPUDevice());
    auto port = importer->getOutputPort();
    importer->update(0);
    auto image = port->getNextFrame<Image>();
    Vector2i size(32, 32);
    Vector2i position = Vector2i(120, 100) + size/2;
    auto templateImage = image->crop(position - size/2, size);

    auto matching = TemplateMatchingNCC::New();
    matching->enableRuntimeMeasurements();
    matching->setRegionOfInterest(position, Vector2i(16, 16));
    matching->setInputData(0, image);
    matching->setInputData(1, templateImage);

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(matching->getOutputPort());
    auto renderer2 = ImageRenderer::New();
    renderer2->addInputData(image);
    auto renderer3 = ImageRenderer::New();
    renderer3->addInputData(templateImage);

    auto window = MultiViewWindow::New();
    window->setNrOfViews(3);
    window->getView(0)->addRenderer(renderer);
    window->getView(0)->set2DMode();
    window->getView(1)->addRenderer(renderer2);
    window->getView(1)->set2DMode();
    window->getView(2)->addRenderer(renderer3);
    window->getView(2)->set2DMode();
    window->setTimeout(1000);
    window->start();

    //matching->update(0);
    matching->getRuntime()->print();

    // Validate position
    REQUIRE(position == matching->getBestFitPixelPosition());
    REQUIRE(position.cast<float>().x() == Approx(matching->getBestFitSubPixelPosition().x()).epsilon(0.1));
    REQUIRE(position.cast<float>().y() == Approx(matching->getBestFitSubPixelPosition().y()).epsilon(0.1));
}

