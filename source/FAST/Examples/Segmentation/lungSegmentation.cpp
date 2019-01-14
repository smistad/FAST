/**
 * Examples/Segmentation/lungSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include <FAST/Exporters/VTKMeshFileExporter.hpp>
#include <FAST/Algorithms/CenterlineExtraction/CenterlineExtraction.hpp>
#include <FAST/Visualization/LineRenderer/LineRenderer.hpp>
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/LungSegmentation/LungSegmentation.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Lung segmentation", "Segments the lungs, airways and blood vessels");
    parser.addPositionVariable(1, "input-filename", Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    parser.addVariable("output-path", false, "Save data to this path in mhd and vtk format");
    parser.addOption("blood-vessel-centerline", "Extract centerline from blood vessels");
    parser.parse(argc, argv);

    // Import image from file using the ImageFileImporter
    auto importer = ImageFileImporter::New();
    importer->setFilename(parser.get("input-filename"));

    // Perform lung segmentation (this will also extract the airways using AirwaySegmentation)
    auto segmentation = LungSegmentation::New();
    segmentation->setInputConnection(importer->getOutputPort());

    auto centerline = CenterlineExtraction::New();
    if(parser.getOption("blood-vessel-centerline")) {
        centerline->setInputConnection(segmentation->getBloodVesselOutputPort());
        centerline->getReporter().setGlobalReportMethod(Reporter::COUT);
    }

    if(parser.gotValue("output-path")) {
        auto exporter = MetaImageExporter::New();
        exporter->setFilename(join(parser.get("output-path"), "vessel_segmentation.mhd"));
        exporter->setInputConnection(segmentation->getBloodVesselOutputPort());

        auto exporter2 = MetaImageExporter::New();
        exporter2->setFilename(join(parser.get("output-path"), "lung_segmentation.mhd"));
        exporter2->setInputConnection(segmentation->getOutputPort(0));

        auto exporter3 = MetaImageExporter::New();
        exporter3->setFilename(join(parser.get("output-path"), "airway_segmentation.mhd"));
        exporter3->setInputConnection(segmentation->getOutputPort(1));

        if(parser.getOption("blood-vessel-centerline")) {
            auto exporter2 = VTKMeshFileExporter::New();
            exporter2->setFilename(join(parser.get("output-path"), "centerline.vtk"));
            exporter2->setInputConnection(centerline->getOutputPort());
            exporter2->update(0);
        }
        exporter->update(0);
        exporter2->update(0);
        exporter3->update(0);
    } else {

        // Extract lung surface
        auto extraction = SurfaceExtraction::New();
        extraction->setInputConnection(segmentation->getOutputPort());

        // Extract airway surface
        auto extraction2 = SurfaceExtraction::New();
        extraction2->setInputConnection(segmentation->getOutputPort(1));

        // Extract blood vessel
        auto extraction3 = SurfaceExtraction::New();
        extraction3->setInputConnection(segmentation->getBloodVesselOutputPort());

        // Render both surfaces with different color
        auto renderer = TriangleRenderer::New();
        renderer->addInputConnection(extraction->getOutputPort(), Color::Green(), 0.6f);
        renderer->addInputConnection(extraction2->getOutputPort(), Color::Red(), 1.0f);
        renderer->addInputConnection(extraction3->getOutputPort(), Color::Blue(), 1.0f);

        auto window = SimpleWindow::New();
        window->addRenderer(renderer);

        if(parser.getOption("blood-vessel-centerline")) {
            auto lineRenderer = LineRenderer::New();
            lineRenderer->addInputConnection(centerline->getOutputPort());
            lineRenderer->setDefaultDrawOnTop(true);
            window->addRenderer(lineRenderer);
        }

#ifdef FAST_CONTINUOUS_INTEGRATION
        // This will automatically close the window after 5 seconds, used for CI testing
        window->setTimeout(5*1000);
#endif
        window->start();
    }
}
