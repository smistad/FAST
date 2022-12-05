/**
 * @example streamUFFData.cpp
 *
 */
#include <FAST/Testing.hpp>
#include <FAST/Streamers/UFFStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget.hpp>
#include <FAST/Algorithms/NonLocalMeans/NonLocalMeans.hpp>
#include <FAST/Algorithms/Interleave/Interleave.hpp>

using namespace fast;

int main(int argc, char** argv) {
	CommandLineParser parser("Stream UFF data");
	parser.addPositionVariable(1, "filename",  Config::getTestDataPath() + "US/UFF/P4_2_A4C.uff", "Path to to UFF file, e.g. /path/to/data.uff");
	parser.addVariable("framerate", "4", "Framerate");
	parser.addOption("loop", "Loop playback");
	parser.parse(argc, argv);

	auto streamer = UFFStreamer::create(
	        parser.get("filename"),
	        parser.getOption("loop"),
	        parser.get<int>("framerate")
    );

	auto nlm = NonLocalMeans::create(3,11,0.25, 0.0)
	        ->connect(streamer);

	auto interleave = InterleavePlayback::create(streamer)
	        ->connect(0, streamer)
	        ->connect(1, nlm);

	auto widget = new PlaybackWidget(interleave);

	auto renderer = ImageRenderer::create()->connect(interleave);

	auto window = SimpleWindow2D::create(Color::Black())
	        ->connect(renderer)
	        ->connect(widget);
    window->run();
}
