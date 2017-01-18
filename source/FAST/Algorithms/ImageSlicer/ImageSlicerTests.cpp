#include "FAST/Testing.hpp"
#include "ImageSlicer.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Image slicer", "[fast][ImageSlicer][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(importer->getOutputPort());
	slicer->setOrthogonalSlicePlane(PLANE_Y);

	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->set2DMode();
	window->setTimeout(1000);
	window->start();
}
