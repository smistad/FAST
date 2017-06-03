#include "FAST/Testing.hpp"
#include "ImageSlicer.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
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

TEST_CASE("Image slicer, arbitrary slice", "[fast][ImageSlicer][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(importer->getOutputPort());
    slicer->setArbitrarySlicePlane(Plane(Vector3f(0, 0, 1)));


	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(importer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(TriangleRenderer);
	window->set3DMode();
	window->setTimeout(1000);
	window->start();
}

TEST_CASE("Image slicer, arbitrary slice 2", "[fast][ImageSlicer][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(importer->getOutputPort());
    slicer->setArbitrarySlicePlane(Plane(Vector3f(0, 1, 1)));


	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(importer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(TriangleRenderer);
	window->set3DMode();
	window->setTimeout(1000);
	window->start();
}

TEST_CASE("Image slicer, arbitrary slice 3", "[fast][ImageSlicer][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(importer->getOutputPort());
    slicer->setArbitrarySlicePlane(Plane(Vector3f(0.5, 0.5, 0.5)));


	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(importer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(TriangleRenderer);
	window->set3DMode();
	window->setTimeout(1000);
	window->start();
}
