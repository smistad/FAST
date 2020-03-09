#include <FAST/Testing.hpp>
#include <FAST/Streamers/UFFStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {
	CommandLineParser parser("Stream UFF data");
	parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "US/b_data_IQ022_A4C.uff");
	parser.addOption("loop", "Loop playback");
	parser.parse(argc, argv);

	auto streamer = UFFStreamer::New();
	streamer->setFilename(parser.get("filename"));
	streamer->setLooping(parser.getOption("loop"));

	auto renderer = ImageRenderer::New();
	renderer->addInputConnection(streamer->getOutputPort());
	renderer->setIntensityLevel(127);
	renderer->setIntensityWindow(255);

	auto window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->set2DMode();
	window->getView()->setBackgroundColor(Color::Black());
	window->start();
}