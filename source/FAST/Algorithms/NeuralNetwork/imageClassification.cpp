#include "FAST/Testing.hpp"
#include "ImageClassifier.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TextRenderer/TextRenderer.hpp"

namespace fast {

    class ClassificationToText : public ProcessObject {
        FAST_OBJECT(ClassificationToText)
    private:
        ClassificationToText() {
            createInputPort<ImageClassification>(0);
            createOutputPort<Text>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
        }
        void execute() {
            ImageClassification::pointer classification = getStaticInputData<ImageClassification>();
            Text::pointer text = getStaticOutputData<Text>();

            // Find classification with max
            ImageClassification::access access = classification->getAccess(ACCESS_READ);
            std::map<std::string, float> values = access->getData();
            float max = 0;
            std::string label;
            for (auto &&item : values) {
                if(item.second > max) {
                    max = item.second;
                    label = item.first;
                }
            }

            Text::access access2 = text->getAccess(ACCESS_READ_WRITE);
            char buffer[8];
            std::sprintf(buffer, "%.2f", max);
            std::string result = label + ": " + buffer;
            access2->setData(result);
        }
    };

}

using namespace fast;


int main() {

    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormats({
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
    streamer->setSleepTime(50);

    ImageClassifier::pointer classifier = ImageClassifier::New();
    classifier->load("/home/smistad/Downloads/cvc_inc_merged");
    classifier->setInputSize(128,128);
    classifier->setOutputParameters({"Softmax"});
    classifier->setLabels({
                                  "Parasternal short axis",
                                  "Parasternal long axis",
                                  "Apical two-chamber",
                                  "Apical five-chamber",
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
    window->setWindowSize(1024, 1024);
    window->addRenderer(textRenderer);
    window->set2DMode();
    window->start();

    classifier->getRuntime()->print();
}