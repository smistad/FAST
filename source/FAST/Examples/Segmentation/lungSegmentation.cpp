/**
 * Examples/Segmentation/lungSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/LungSegmentation/LungSegmentation.hpp"


using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Lung segmentation", "Segments the lungs and airways");
    parser.addPositionVariable(1, "input-filename", Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    parser.addVariable("output-filename", false, "Filename of where to export filename (mhd format). Example: /path/to/filename.mhd");
    parser.parse(argc, argv);

    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(parser.get("input-filename"));

    // Perform lung segmentation (this will also extract the airways using AirwaySegmentation)
    LungSegmentation::pointer segmentation = LungSegmentation::New();
    segmentation->setInputConnection(importer->getOutputPort());

    if(parser.gotValue("output-filename")) {
        MetaImageExporter::pointer exporter = MetaImageExporter::New();
        exporter->setFilename(parser.get("output-filename"));
        exporter->setInputConnection(segmentation->getOutputPort());
        exporter->update(0);
    }

    // Extract lung surface
    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());

    // Extract airway surface
    SurfaceExtraction::pointer extraction2 = SurfaceExtraction::New();
    extraction2->setInputConnection(segmentation->getOutputPort(1));

    // Extract blood vessel
    SurfaceExtraction::pointer extraction3 = SurfaceExtraction::New();
    extraction3->setInputConnection(segmentation->getBloodVesselOutputPort());

    // Render both surfaces with different color
    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort(), Color::Green(), 0.6f);
    TriangleRenderer->addInputConnection(extraction2->getOutputPort(), Color::Red(), 1.0f);
    TriangleRenderer->addInputConnection(extraction3->getOutputPort(), Color::Blue(), 1.0f);

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(TriangleRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
