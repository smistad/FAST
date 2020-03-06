#include <FAST/Testing.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include "ImageWeightedMovingAverage.hpp"

using namespace fast;

TEST_CASE("Image weighted moving average 2D", "[fast][ImageWeightedMovingAverage][visual]") {
   auto streamer = ImageFileStreamer::New();
   streamer->setFilenameFormat(Config::getTestDataPath() + "US/Axillary/US-2D_#.mhd");
   streamer->setSleepTime(200);

   auto thresholding = BinaryThresholding::New();
   thresholding->setLowerThreshold(100);
   thresholding->setInputConnection(streamer->getOutputPort());

   auto average = ImageWeightedMovingAverage::New();
   average->setInputConnection(thresholding->getOutputPort());
   average->setFrameCount(15);

   auto renderer = ImageRenderer::New();
   renderer->addInputConnection(average->getOutputPort());
   renderer->setIntensityLevel(0.5);
   renderer->setIntensityWindow(1);

   auto window = SimpleWindow::New();
   window->addRenderer(renderer);
   window->set2DMode();
   window->setTimeout(3000);
   window->start();
}