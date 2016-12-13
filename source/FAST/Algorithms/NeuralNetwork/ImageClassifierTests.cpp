#include "FAST/Testing.hpp"
#include "ImageClassifier.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("Image classifier", "[fast][ImageClassifier]") {

	Reporter::setGlobalReportMethod(Reporter::COUT);
	ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename("/media/extra/GRUE_images/Clinic001/F4AFP0HG/US-2D_0.png");

	ImageClassifier::pointer classifier = ImageClassifier::New();
	classifier->load("/home/smistad/Downloads/cvc_inc_merged");
    classifier->setInputParameters("input", 128,128);
	classifier->setOutputParameters({"Softmax"});
	//classifier->setLabels({"Not vessel", "Vessel"});
	classifier->setInputConnection(importer->getOutputPort());
	classifier->enableRuntimeMeasurements();
	classifier->update();
	classifier->getRuntime()->print();
}
