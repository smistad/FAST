#include "GUI.hpp"
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {
	CommandLineParser parser("Ultasound File Format FAST Viewer");
	parser.addPositionVariable(1, "filename", false, "Path to uff file to open");
	parser.addOption("verbose");
	parser.parse(argc, argv);

	if(parser.getOption("verbose"))
		Reporter::setGlobalReportMethod(Reporter::COUT);

	auto window = UFFViewerWindow::New();
	if(parser.gotValue("filename")) {
		window->setFilename(parser.get("filename"));
	}
	window->start();
}