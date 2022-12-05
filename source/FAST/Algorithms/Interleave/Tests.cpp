#include <FAST/Testing.hpp>
#include "Interleave.hpp"
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>

using namespace fast;

TEST_CASE("Alternating output", "[fast][Interleave][visual]") {
    auto streamer = ImageFileStreamer::create(
            Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd"
            );

    auto smoothing = GaussianSmoothing::create(2.0f)
            ->connect(streamer);
    auto smoothing2 = GaussianSmoothing::create(4.0f)
            ->connect(streamer);

    auto interleave = Interleave::create(5)
            ->connect(0, streamer)
            ->connect(1, smoothing)
            ->connect(2, smoothing2);

    auto enhance = UltrasoundImageEnhancement::create()->connect(interleave);

    auto renderer = ImageRenderer::create()
            ->connect(enhance);

    auto window = SimpleWindow2D::create()
            ->connect(renderer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Alternating output data stream", "[fast][Interleave][visual]") {
    auto streamer = ImageFileStreamer::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");

    auto smoothing = GaussianSmoothing::create(2.0f)
            ->connect(streamer);

    auto interleave = Interleave::create(30)
            ->connect(0, streamer)
            ->connect(1, smoothing);

    auto enhance = UltrasoundImageEnhancement::create()->connect(interleave);

    auto stream = DataStream(enhance);

    do {
        auto image = stream.getNextFrame<Image>();
        std::cout << image->calculateAverageIntensity() << std::endl;
    } while(!stream.isDone());
}