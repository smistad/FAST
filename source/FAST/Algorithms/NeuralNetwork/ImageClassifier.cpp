#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {

}

void ImageClassifier::setLabels(std::vector<std::string> labels) {
	mLabels = labels;
}

std::vector<std::map<std::string, float> > ImageClassifier::getClassification(std::string outputLayerName) {
    // Read output layer
	std::vector<float> result = getNetworkOutput();

	reportInfo() << "RESULT: " << reportEnd();
    std::vector<std::map<std::string, float> > labelsAndScores;
	int batch_size, channels;
	if(outputLayerName == "") {
        if(mNetwork->num_outputs() == 0)
			throw Exception("No outputs found in the network.");
		caffe::Blob<float>* output_layer = mNetwork->output_blobs()[0];
		batch_size = output_layer->num();
        channels = output_layer->channels();
	} else {
        if(!mNetwork->has_blob(outputLayerName))
			throw Exception("Blob with name " + outputLayerName + " not found in the network.");
        boost::shared_ptr<caffe::Blob<float> > output_layer = mNetwork->blob_by_name(outputLayerName);
		batch_size = output_layer->num();
		channels = output_layer->channels();
	}
	int k = 0;
	for(int i = 0; i < batch_size; ++i) { // input images
		std::map<std::string, float> mapResult;
		for(int j = 0; j < channels; ++j) { // classes
			mapResult[mLabels[j]] = result[k];
			reportInfo() << mLabels[j] << ": " << result[k] << reportEnd();
			++k;
		}
		labelsAndScores.push_back(mapResult);
	}
	return labelsAndScores;
}

void ImageClassifier::execute() {
	NeuralNetwork::execute();
}

}
