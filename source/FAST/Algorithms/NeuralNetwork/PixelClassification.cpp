#include "PixelClassification.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

PixelClassification::PixelClassification() {
    createInputPort<Image>(0);

    mNrOfClasses = -1;
}

void PixelClassification::setNrOfClasses(uint classes) {
    mNrOfClasses = classes;
    for(int i = 0; i < mNrOfClasses; i++) {
        createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    }
}

void PixelClassification::execute() {
    if(mNrOfClasses <= 0) {
        throw Exception("You must set the nr of classes to pixel classification.");
    }

    std::vector<tensorflow::Tensor> result;
    result = getNetworkOutput("Sigmoid");

	for(int i = 0; i < result.size(); ++i) { // For each input image
        tensorflow::Tensor tensor = result[i];
        // For each class
        for(int j = 0; j < mNrOfClasses; j++) {
            Image::pointer output = getStaticOutputData<Image>(j);
            output->create(64, 64, TYPE_FLOAT, 1, result[i]);
        }
	}

}

}