#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageRenderer with single 2D image in 2D mode", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(500);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with dynamic 2D image in 2D mode", "[fast][ImageRenderer][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_#.mhd");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(streamer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with single 3D image in 2D mode and Axial plane", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"CT-Abdomen.mhd");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->getView()->setViewingPlane(Plane::Axial());
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with single 3D image in 2D mode and Coronal plane", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"CT-Abdomen.mhd");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->getView()->setViewingPlane(Plane::Coronal());
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with single 3D image in 2D mode in Sagittal plane", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"CT-Abdomen.mhd");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->getView()->setViewingPlane(Plane::Sagittal());
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with single 3D image in 2D mode and custom plane", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"CT-Abdomen.mhd");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->getView()->setViewingPlane(Plane(Vector3f(0.5, 0.5, 0.5)));
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

