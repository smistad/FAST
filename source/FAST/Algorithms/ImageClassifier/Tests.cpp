#include "FAST/Testing.hpp"
#include "ImageClassifier.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("Image classifier", "[fast][ImageClassifier]") {

	std::string modelFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/deploy.prototxt";
	std::string trainingFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/snapshot_iter_90.caffemodel";
	std::string meanFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/mean.binaryproto";

	Reporter::setGlobalReportMethod(Reporter::COUT);
	ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename("/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/image64x64.png");

	ImageClassifier::pointer classifier = ImageClassifier::New();
	classifier->loadModel(modelFile, trainingFile, meanFile);
	classifier->setLabels({"Not vessel", "Vessel"});
	classifier->setInputConnection(importer->getOutputPort());
	classifier->enableRuntimeMeasurements();
	classifier->update();
	classifier->getRuntime()->print();
}
