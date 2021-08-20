#include <FAST/Streamers/ImageFileStreamer.hpp>
#include "FAST/Testing.hpp"
#include "ImageSlicer.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Image slicer", "[fast][ImageSlicer][visual]") {
	auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

	auto slicer = ImageSlicer::create(PLANE_Y)->connect(importer);

	auto renderer = ImageRenderer::create()->connect(slicer);

	auto window = SimpleWindow2D::create()->connect(renderer);
	window->setTimeout(1000);
	window->run();
}

TEST_CASE("Image slicer arbitrary slice", "[fast][ImageSlicer][visual][asdasd]") {
	auto importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	auto slicer = ImageSlicer::create(Plane(Vector3f(0, 0, 1)))->connect(importer);

	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::create();
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

	SurfaceExtraction::pointer extraction = SurfaceExtraction::create();
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

	SurfaceExtraction::pointer extraction = SurfaceExtraction::create();
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

TEST_CASE("Image slicer, multiple slices", "[fast][ImageSlicer][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    ImageSlicer::pointer slicer = ImageSlicer::New();
    slicer->setInputConnection(importer->getOutputPort());
	slicer->setOrthogonalSlicePlane(PLANE_Y);

	ImageSlicer::pointer slicer2 = ImageSlicer::New();
	slicer2->setInputConnection(importer->getOutputPort());
	slicer2->setOrthogonalSlicePlane(PLANE_X);

	ImageSlicer::pointer slicer3 = ImageSlicer::New();
	slicer3->setInputConnection(importer->getOutputPort());
	slicer3->setOrthogonalSlicePlane(PLANE_Z);

	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());
	renderer->addInputConnection(slicer2->getOutputPort());
	renderer->addInputConnection(slicer3->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->set3DMode();
	window->setTimeout(1000);
	window->start();
}

TEST_CASE("Image slicer, multiple slices, streaming", "[fast][ImageSlicer][streaming][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/Ball/US-3Dt_#.mhd");
	streamer->enableLooping();

    ImageSlicer::pointer slicer = ImageSlicer::New();
    //slicer->setInputConnection(importer->getOutputPort());
	slicer->setInputConnection(streamer->getOutputPort());
	slicer->setOrthogonalSlicePlane(PLANE_Y);

	ImageSlicer::pointer slicer2 = ImageSlicer::New();
	//slicer2->setInputConnection(importer->getOutputPort());
	slicer2->setInputConnection(streamer->getOutputPort());
	slicer2->setOrthogonalSlicePlane(PLANE_X);

	ImageSlicer::pointer slicer3 = ImageSlicer::New();
	//slicer3->setInputConnection(importer->getOutputPort());
	slicer3->setInputConnection(streamer->getOutputPort());
	slicer3->setOrthogonalSlicePlane(PLANE_Z);

	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());
	renderer->addInputConnection(slicer2->getOutputPort());
	renderer->addInputConnection(slicer3->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->set3DMode();
	window->setTimeout(2000);
	window->start();
}