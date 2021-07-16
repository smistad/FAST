#include <FAST/Testing.hpp>
#include <FAST/Algorithms/ImageSharpening/ImageSharpening.hpp>
#include <FAST/Importers/ImageImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>

using namespace fast;

TEST_CASE("Image sharpen", "[fast][ImageSharpening][visual]") {
	auto importer = WholeSlideImageImporter::New();
	importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

	auto tissueSegmentation = TissueSegmentation::New();
	tissueSegmentation->setInputConnection(importer->getOutputPort());

	auto generator = PatchGenerator::New();
	generator->setPatchSize(512, 512);
	generator->setInputConnection(importer->getOutputPort());
	generator->setInputConnection(1, tissueSegmentation->getOutputPort());

	auto port =	generator->getOutputPort();
	generator->update();
	auto image = port->getNextFrame<Image>();
	generator->stop();

	auto filter = ImageSharpening::create(1.0f, 1.5f)->connect(image);
	filter->enableRuntimeMeasurements();

	auto renderer = ImageRenderer::create()->connect(image);

	auto renderer2 = ImageRenderer::create()->connect(filter);

	auto window = DualViewWindow::create()
	        ->connectLeft(renderer)
	        ->connectRight(renderer2);
	window->set2DMode();
	window->setTimeout(2000);
	window->run();
	filter->getRuntime()->print();

}