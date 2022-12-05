#include <FAST/Streamers/OpenIGTLinkStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {

	CommandLineParser parser("OpenIGTLink streaming example");
	parser.addPositionVariable(1, "address", "localhost", "Address of OpenIGTLink server to connect to.");
	parser.addVariable("device-name", "default", "Device name to stream. If \"default\", one IMAGE stream will be selected");

	parser.parse(argc, argv);

	Reporter::setGlobalReportMethod(Reporter::COUT);
	auto streamer = OpenIGTLinkStreamer::create(parser.get("address"));

	auto deviceName = parser.get("device-name");
	deviceName = deviceName == "default" ? "" : deviceName;
	auto renderer = ImageRenderer::create();
	renderer->connect(streamer, streamer->getOutputPortNumber(deviceName));

	SimpleWindow2D::create()
		->connect(renderer)
		->run();
}