/**
 * Examples/Segmentation/airwaySegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Algorithms/AirwaySegmentation/AirwaySegmentation.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/CenterlineExtraction/CenterlineExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
	// Import CT data
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

	// Do airway segmenetation
	AirwaySegmentation::pointer segmentation = AirwaySegmentation::New();
	segmentation->setInputConnection(importer->getOutputPort());

	// Extract centerline from segmentation
	CenterlineExtraction::pointer centerline = CenterlineExtraction::New();
	centerline->setInputConnection(segmentation->getOutputPort());
	centerline->update();

	// Extract surface from segmentation
	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(segmentation->getOutputPort());

	// Set up renderers and window
	TriangleRenderer::pointer renderer = TriangleRenderer::New();
	renderer->addInputConnection(extraction->getOutputPort());

	LineRenderer::pointer lineRenderer = LineRenderer::New();
	lineRenderer->addInputConnection(centerline->getOutputPort());
	lineRenderer->setDefaultDrawOnTop(true);

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(lineRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
	window->start();
}
