#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp>
#include <FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Algorithms/SegmentationVolumeReconstructor/SegmentationVolumeReconstructor.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

using namespace fast;

TEST_CASE("Segmentation volume reconstructor", "[SegmentationVolumeReconstructor][fast][visual]") {
    Reporter::setGlobalReportMethod(Reporter::NONE);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd");
    streamer->setTimestampFilename(Config::getTestDataPath() + "US/JugularVein/timestamps.fts");
    //streamer->setFilenameFormat("/home/smistad/data/2018-05-23_ROMO_Phantom_test.cx3/US_Acq/US-Acq_1_20180523T132954/US-Acq_1_20180523T132954_Sonix_#.mhd");
    //streamer->setTimestampFilename("/home/smistad/data/2018-05-23_ROMO_Phantom_test.cx3/US_Acq/US-Acq_1_20180523T132954/US-Acq_1_20180523T132954_Sonix.fts");

    auto segmentation = SegmentationNetwork::New();
    segmentation->load(Config::getTestDataPath() + "NeuralNetworkModels/jugular_vein_segmentation.pb");
    segmentation->setScaleFactor(1.0f / 255.0f);
    segmentation->setInputConnection(streamer->getOutputPort());

    SegmentationVolumeReconstructor::pointer reconstructor = SegmentationVolumeReconstructor::New();
    reconstructor->setInputConnection(segmentation->getOutputPort());

    SurfaceExtraction::pointer extraction = SurfaceExtraction::create();
    extraction->setInputConnection(reconstructor->getOutputPort());

    TriangleRenderer::pointer renderer = TriangleRenderer::New();
    renderer->addInputConnection(extraction->getOutputPort());

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(imageRenderer);
    window->setTimeout(1000);
    window->start();
}
