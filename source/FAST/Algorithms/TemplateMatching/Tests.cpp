#include <FAST/Testing.hpp>
#include "TemplateMatching.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

using namespace fast;

TEST_CASE("Template matching NCC", "[fast][NCC][TemplateMatching][visual]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(join(Config::getTestDataPath(), "US/CarotidArtery/Left/US-2D_300.mhd"));
    importer->setMainDevice(DeviceManager::getInstance()->getOneGPUDevice());
    auto port = importer->getOutputPort();
    importer->update();
    auto image = port->getNextFrame<Image>();
    Vector2i size(33, 33);
    Vector2i position = Vector2i(120, 100) + size/2;
    auto templateImage = image->crop(position - size/2, size);

    auto matching = TemplateMatching::create(MatchingMetric::NORMALIZED_CROSS_CORRELATION, position, Vector2i(16, 16))->connect(image)->connect(1, templateImage);
    matching->enableRuntimeMeasurements();

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(matching->getOutputPort());
    auto renderer2 = ImageRenderer::New();
    renderer2->addInputData(image);
    auto renderer3 = ImageRenderer::New();
    renderer3->addInputData(templateImage);

    auto window = MultiViewWindow::create(3)
            ->connect(0, renderer)
            ->connect(1, renderer2)
            ->connect(2, renderer3);
    window->set2DMode();
    window->setTimeout(1000);
    window->run();

    //matching->update();
    matching->getRuntime()->print();

    // Validate position
    REQUIRE(position == matching->getBestFitPixelPosition());
    REQUIRE(std::fabs(position.cast<float>().x() - matching->getBestFitSubPixelPosition().x()) < 0.1);
    REQUIRE(std::fabs(position.cast<float>().y() - matching->getBestFitSubPixelPosition().y()) < 0.1);
}


/*
TEST_CASE("Template matching NCC synthetic sequence", "[fast][NCC][TemplateMatching]") {
    // Import first frame
    Vector2i size(72, 72);
    Vector2i position = Vector2i(4, 4) + size / 2;
    Image::pointer templateImage;
    {
        auto importer = ImageFileImporter::New();
        importer->setFilename(join(Config::getTestDataPath(), "Synthetic/ImageTracking/frame_0.png"));
        importer->setMainDevice(DeviceManager::getInstance()->getOneGPUDevice());
        auto port = importer->getOutputPort();
        importer->update();
        auto image = port->getNextFrame<Image>();
        templateImage = image->crop(position - size / 2, size);
    }

    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(join(Config::getTestDataPath(), "Synthetic/ImageTracking/frame_#.png"));
    //streamer->setMainDevice(DeviceManager::getInstance()->getOneGPUDevice());

    auto matching = TemplateMatching::New();
    matching->getReporter().setReportMethod(Reporter::NONE);
    matching->setInputData(1, templateImage);
    matching->setInputConnection(0, streamer->getOutputPort());
    Vector2f previousRealPosition = position.cast<float>();
    // Actual speed is 0.5 pixels x, 0.25 pixels y
    for(int i = 0; i < streamer->getNrOfFrames(); ++i) {
        matching->setRegionOfInterest(position, Vector2i(2, 2));
        matching->update();
        //std::cout << "frame: " << i << std::endl;
        Vector2f newPosition = matching->getBestFitSubPixelPosition();
        //std::cout << "new pos: " << newPosition.transpose() << std::endl;
        Vector2f movement = newPosition - previousRealPosition;
        if(i > 0) {
            REQUIRE(std::fabs(movement.x() - 0.5f) < 0.01);
            REQUIRE(std::fabs(movement.y() - 0.25f) < 0.1);
        }
        //std::cout << (newPosition - previousRealPosition).x() << std::endl;
        //std::cout << (newPosition - previousRealPosition).y() << std::endl;
        previousRealPosition = newPosition;

        // Convert to pixel position
        newPosition.x() = fast::round(newPosition.x());
        newPosition.y() = fast::round(newPosition.y());
        position = newPosition.cast<int>();
    }
}
 */
