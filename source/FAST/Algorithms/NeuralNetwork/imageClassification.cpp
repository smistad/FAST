#include "FAST/Testing.hpp"
#include "ImageClassifier.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TextRenderer/TextRenderer.hpp"

using namespace fast;

int main() {

    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormats({
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADB20I/US-2D_#.mhd",
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADBNGK/US-2D_#.mhd",
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1ADC6OM/US-2D_#.mhd",
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1AD8S80/US-2D_#.mhd",
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9B04/US-2D_#.mhd",
         "/home/smistad/data/ultrasound_smistad_heart/1234/H1AD9EG6/US-2D_#.mhd",
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
    });
    streamer->enableLooping();
    streamer->setSleepTime(25);
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    ImageClassifier::pointer classifier = ImageClassifier::New();
    classifier->setScaleFactor(1.0f/255.0f);
    classifier->load("/home/smistad/Downloads/cvc_net");
    classifier->setInputSize(128,128);
    classifier->setOutputParameters({"Softmax"});
    classifier->setLabels({
                              "Parasternal short axis",
                              "Parasternal long axis",
                              "Apical two-chamber",
                              "Apical four-chamber",
                              "Apical long axis"
                      });
    classifier->setInputConnection(streamer->getOutputPort());
    classifier->enableRuntimeMeasurements();

    ClassificationToText::pointer classToText = ClassificationToText::New();
    classToText->setInputConnection(classifier->getOutputPort());

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();

    TextRenderer::pointer textRenderer = TextRenderer::New();
    textRenderer->setView(window->getView());
    textRenderer->setPosition(Vector2i(10, 40));
    textRenderer->setFontSize(32);
    textRenderer->setInputConnection(classToText->getOutputPort());

    window->addRenderer(renderer);
    window->addRenderer(textRenderer);
    window->setSize(1024, 1024);
    //window->enableFullscreen();
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    classifier->getRuntime()->print();
    classifier->getRuntime("input_data_copy")->print();
    classifier->getRuntime("network_execution")->print();
}
