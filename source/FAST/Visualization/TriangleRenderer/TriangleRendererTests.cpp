#include <FAST/Algorithms/ImageSlicer/ImageSlicer.hpp>
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "TriangleRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Mesh.hpp"

using namespace fast;

TEST_CASE("TriangleRenderer on LV surface model", "[fast][TriangleRenderer][visual]") {
    CHECK_NOTHROW(
        VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
        TriangleRenderer::pointer renderer = TriangleRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("TriangleRenderer on stream of surfaces", "[fast][TriangleRenderer][visual]") {
    CHECK_NOTHROW(
        ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
        mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
        SurfaceExtraction::pointer extractor = SurfaceExtraction::create();
        extractor->setInputConnection(mhdStreamer->getOutputPort());
        extractor->setThreshold(200);
        TriangleRenderer::pointer renderer = TriangleRenderer::New();
        renderer->addInputConnection(extractor->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("TriangleRenderer in 2D mode", "[fast][TriangleRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(importer->getOutputPort());
    slicer->setOrthogonalSlicePlane(PLANE_Z);

	ImageRenderer::pointer renderer = ImageRenderer::New();
	renderer->addInputConnection(slicer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::create();
	extraction->setInputConnection(importer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->setTimeout(1000);
	window->start();

}
