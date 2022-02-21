#include <FAST/Streamers/OpenIGTLinkStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {

	CommandLineParser parser("OpenIGTLink streaming example");
	parser.addPositionVariable(1, "address", "localhost", "Address of OpenIGTLink server to connect to.");

	parser.parse(argc, argv);

	Reporter::setGlobalReportMethod(Reporter::COUT);
	auto streamer = OpenIGTLinkStreamer::create(parser.get("address"));

//	auto renderer = ImageRenderer::create()
//		->connect(streamer, streamer->getOutputPortNumber("BMode"));
	auto renderer2 = ImageRenderer::create()
		->connect(streamer);

	SimpleWindow2D::create()
		->connect( renderer2 )
		->run();
}