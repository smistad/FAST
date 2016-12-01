#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {
	createOutputPort<ImageClassification>(0, OUTPUT_DEPENDS_ON_INPUT, 0, true);

	mOutputName = "";
}

void ImageClassifier::setLabels(std::vector<std::string> labels) {
	mLabels = labels;
}
void ImageClassifier::setOutputName(std::string outputName)	 {
	mOutputName = outputName;
}

void ImageClassifier::execute() {
	NeuralNetwork::execute();

	// Create output data
    // Read output layer
	std::vector<float> result = getNetworkOutput();

	reportInfo() << "RESULT: " << reportEnd();
	int batch_size, channels;
	if(mOutputName == "") {
        if(mNetwork->num_outputs() == 0)
			throw Exception("No outputs found in the network.");
		caffe::Blob<float>* output_layer = mNetwork->output_blobs()[0];
		batch_size = output_layer->num();
        channels = output_layer->channels();
	} else {
        if(!mNetwork->has_blob(mOutputName))
			throw Exception("Blob with name " + mOutputName + " not found in the network.");
        boost::shared_ptr<caffe::Blob<float> > output_layer = mNetwork->blob_by_name(mOutputName);
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

		ImageClassification::pointer result = addStaticOutputData<ImageClassification>(0);
		result->create(mapResult);
	}

}

}
