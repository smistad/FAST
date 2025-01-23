/**
 * @example importWholeSlideImage.cpp
 * An example of importing and visualizing an image from file using the WholeSlideImageImporter
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

using namespace fast;

int main(int argc, char** argv) {
    //Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Import image from file example");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "/WSI/CMU-1.svs");
    parser.addOption("scalebar", "Show scalebar");
    parser.addOption("tissue-segmentation", "Do tissue segmentation");
    parser.parse(argc, argv);

    auto importer = WholeSlideImageImporter::create(parser.get(1));

    auto renderer = ImagePyramidRenderer::create()->connect(importer);

    auto window = SimpleWindow2D::create()->connect(renderer);

    if(parser.getOption("tissue-segmentation")) {
        auto tissue = TissueSegmentation::create(false, -1, 1.25)->connect(importer);
        auto segRenderer = SegmentationRenderer::create()->connect(tissue);
        segRenderer->setOpacity(0.4, 0.9);
        window->connect(segRenderer);
    }
    if(parser.getOption("scalebar")) {
        window->getView()->setScalebar(true);
    }
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    // Run entire pipeline and display window
    window->run();
}
