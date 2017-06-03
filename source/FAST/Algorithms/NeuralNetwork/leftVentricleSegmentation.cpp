#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include "FAST/Testing.hpp"
#include "ShapeRegressor.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "PixelClassifier.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormats({
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT5OE/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT6GG/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT7OK/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT50C/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT70I/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT80M/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT88O/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KT98S/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTA0U/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTD1I/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTG1S/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTGPU/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTHA0/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTI22/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTII4/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTJ26/US-2D_#.mhd",
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTJQ8/US-2D_#.mhd",
                                         /*
                                         "/media/extra/GRUE_MHD/Clinic007/F47KTL2C/US-2D_#.mhd",
                                     //            "/media/extra/GRUE_MHD/Clinic002/F4HESG80/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ2LM/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ3TQ/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ4LS/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ6M2/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ7U6/US-2D_#.mhd",
                                     //                    "/media/extra/GRUE_MHD/Clinic002/F47KQ9O4/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1ADB20I/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADBNGK/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADC6OM/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1AD8S80/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9B04/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9EG6/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9L08/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9282/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1ADA3OA/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1ADAK8C/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1ADAKGE/US-2D_#.mhd",
                                              //"/home/smistad/data/ultrasound_smistad_heart/1234/H1ADB1GG/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADCKOO/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADCL8Q/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADFN0S/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADFNGU/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADG510/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADGT12/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADGT94/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADI696/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADJ198/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADJ198/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADJG1C/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADJIHE/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADJJ1G/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADK11I/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADKQ9K/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADKQPM/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADLMPO/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADLPPQ/US-2D_#.mhd",
                                              "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADLS9S/US-2D_#.mhd",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP0HG/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP2HM/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP3PQ/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP4PU/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP5I0/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP6A2/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP7I4/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP8I6/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP39O/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFP49S/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFPE2U/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFPIBA/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFPIRC/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFPLBO/US-2D_#.png",
                                              "/media/extra/GRUE_images/Clinic001/F4AFPMJQ/US-2D_#.png",
                                              */
    });
    streamer->enableLooping();
    streamer->setSleepTime(50);
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    PixelClassifier::pointer segmentation = PixelClassifier::New();
    segmentation->setNrOfClasses(4);
    segmentation->load("/home/smistad/workspace/acnn-heart-segmentation/models/tensorflow_segmentation_model.pb");
    segmentation->setInputSize(256, 256);
    segmentation->setScaleFactor(1.0f/255.0f);
    segmentation->setOutputParameters({"conv2d_23/truediv"});
    segmentation->setInputConnection(streamer->getOutputPort());
    segmentation->setHeatmapOutput();
    segmentation->enableRuntimeMeasurements();

    SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->setFillArea(false);
    segmentationRenderer->setInputConnection(segmentation->getOutputPort(1));

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->setInputConnection(streamer->getOutputPort());

    HeatmapRenderer::pointer heatmapRenderer = HeatmapRenderer::New();
    heatmapRenderer->addInputConnection(segmentation->getOutputPort(1), Color::Green());
    heatmapRenderer->addInputConnection(segmentation->getOutputPort(2), Color::Blue());
    heatmapRenderer->addInputConnection(segmentation->getOutputPort(3), Color::Red());

    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(imageRenderer);
    //window->addRenderer(segmentationRenderer);
    window->addRenderer(heatmapRenderer);
    window->setSize(1024, 1024);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    segmentation->getRuntime()->print();
    segmentation->getRuntime("input_data_copy")->print();
    segmentation->getRuntime("network_execution")->print();
}