#include "FAST/Testing.hpp"
#include "ImageClassifier.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("Image classifier", "[fast][ImageClassifier]") {
	Reporter::setGlobalReportMethod(Reporter::COUT);
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename("/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/image64x64.png");

	ImageClassifier::pointer classifier = ImageClassifier::New();
	classifier->setInputConnection(importer->getOutputPort());
	classifier->update();
}
