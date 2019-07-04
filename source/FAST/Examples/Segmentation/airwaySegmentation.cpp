/**
 * Examples/Segmentation/airwaySegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Exporters/MetaImageExporter.hpp>
#include <FAST/Exporters/VTKMeshFileExporter.hpp>
#include "FAST/Algorithms/AirwaySegmentation/AirwaySegmentation.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/CenterlineExtraction/CenterlineExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Tools/CommandLineParser.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT); // TODO remove
	CommandLineParser parser("Airway segmentation");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    parser.addVariable("smoothing", "0.5", "How much smoothing to apply before segmentation");
    parser.addVariable("seed", false, "Manual seed point coordinate (--seed x,y,z)");
    parser.addVariable("export_segmentation", false, "Filename to export segmentation volume to. Example: segmentation.mhd");
    parser.addVariable("export_centerline", false, "Filename to export centerline to. Example: centerline.vtk");
	parser.parse(argc, argv);

	// Import CT data
	auto importer = ImageFileImporter::New();
	importer->setFilename(parser.get(1));

	// Do airway segmentation
	auto segmentation = AirwaySegmentation::New();
	segmentation->setInputConnection(importer->getOutputPort());
	segmentation->setSmoothing(parser.get<float>("smoothing"));
	if(parser.gotValue("seed"))
        segmentation->setSeedPoint(parser.get<Vector3i>("seed"));

	// Extract centerline from segmentation
	auto centerline = CenterlineExtraction::New();
	centerline->setInputConnection(segmentation->getOutputPort());

    // Export data if user has specified to do so
    if(parser.gotValue("export_segmentation")) {
        auto exporter = MetaImageExporter::New();
        exporter->setFilename(parser.get("export_segmentation"));
        exporter->setInputConnection(segmentation->getOutputPort());
        exporter->update();
    }
    if(parser.gotValue("export_centerline")) {
        auto exporter = VTKMeshFileExporter::New();
        exporter->setFilename(parser.get("export_centerline"));
        exporter->setInputConnection(centerline->getOutputPort());
        exporter->update();
		
    }
	if(parser.gotValue("export_segmentation") || parser.gotValue("export_centerline"))
		return 0; // Dont render if exporting..

	// Extract surface from segmentation
	auto extraction = SurfaceExtraction::New();
	extraction->setInputConnection(segmentation->getOutputPort());

	// Set up renderers and window
	auto renderer = TriangleRenderer::New();
	renderer->addInputConnection(extraction->getOutputPort());

	auto lineRenderer = LineRenderer::New();
	lineRenderer->addInputConnection(centerline->getOutputPort());
	lineRenderer->setDefaultDrawOnTop(true);
	lineRenderer->setDefaultColor(Color::Blue());

	auto window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->addRenderer(lineRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
	window->start();


}
