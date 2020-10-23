#include "GUI.hpp"
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {
	CommandLineParser parser("Ultasound File Format FAST Viewer");
	parser.addPositionVariable(1, "filename", false, "Path to uff file to open");
	parser.parse(argc, argv);

	auto window = UFFViewerWindow::New();
	if(parser.gotValue("filename")) {
		window->setFilename(parser.get("filename"));
	}
	window->start();
}