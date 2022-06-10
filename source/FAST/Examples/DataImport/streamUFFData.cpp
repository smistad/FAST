/**
 * @example streamUFFData.cpp
 *
 */
#include <FAST/Testing.hpp>
#include <FAST/Streamers/UFFStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {
	CommandLineParser parser("Stream UFF data");
	parser.addPositionVariable(1, "filename",  Config::getTestDataPath() + "US/UFF/P4_2_A4C.uff", "Path to to UFF file, e.g. /path/to/data.uff");
	parser.addVariable("framerate", "10", "Framerate");
	parser.addOption("loop", "Loop playback");
	parser.parse(argc, argv);

	auto streamer = UFFStreamer::create(
	        parser.get("filename"),
	        parser.getOption("loop"),
	        parser.get<int>("framerate")
    );

	auto renderer = ImageRenderer::create()->connect(streamer);

	auto window = SimpleWindow2D::create(Color::Black())
	        ->connect(renderer);
    window->run();
}
