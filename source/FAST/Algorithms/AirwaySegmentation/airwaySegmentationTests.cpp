#include "FAST/Testing.hpp"
#include "AirwaySegmentation.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/CenterlineExtraction/CenterlineExtraction.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

/*
TEST_CASE("Airway segmentation ALL", "[fast][AirwaySegmentation]") {
	Reporter::setGlobalReportMethod(Reporter::COUT);
	for(int i = 1; i < 26; ++i) {
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        //importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "/CT/CT-Thorax.mhd");
        std::string nr = boost::lexical_cast<std::string>(i);
        if(nr.size() == 1) {
        	nr = "0" + nr;
        }
        importer->setFilename("/home/smistad/Data/lunge_datasett/pasient" + nr + ".mhd");

        AirwaySegmentation::pointer segmentation = AirwaySegmentation::New();
        segmentation->setInputConnection(importer->getOutputPort());

        CenterlineExtraction::pointer centerline = CenterlineExtraction::New();
        centerline->setInputConnection(segmentation->getOutputPort());
        centerline->update();

        SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
        extraction->setInputConnection(segmentation->getOutputPort());

        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->addInputConnection(extraction->getOutputPort());

        LineRenderer::pointer lineRenderer = LineRenderer::New();
        lineRenderer->addInputConnection(centerline->getOutputPort());
        lineRenderer->setDefaultDrawOnTop(true);

        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->addRenderer(lineRenderer);
        window->start();
	}
}
*/

TEST_CASE("Airway segmentation", "[fast][AirwaySegmentation][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT/CT-Thorax.mhd");

	AirwaySegmentation::pointer segmentation = AirwaySegmentation::New();
	segmentation->setInputConnection(importer->getOutputPort());
	segmentation->enableRuntimeMeasurements();

	CenterlineExtraction::pointer centerline = CenterlineExtraction::New();
	centerline->setInputConnection(segmentation->getOutputPort());
	centerline->update();

	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(segmentation->getOutputPort());

	MeshRenderer::pointer renderer = MeshRenderer::New();
	renderer->addInputConnection(extraction->getOutputPort());

	LineRenderer::pointer lineRenderer = LineRenderer::New();
	lineRenderer->addInputConnection(centerline->getOutputPort());
	lineRenderer->setDefaultDrawOnTop(true);

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(lineRenderer);
	window->setTimeout(1000);
    CHECK_NOTHROW(window->start());
	//segmentation->getRuntime()->print();
}
