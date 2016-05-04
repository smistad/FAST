#ifndef IMAGE_CLASSIFIER_HPP
#define IMAGE_CLASSIFIER_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class ImageClassifier : public ProcessObject {
	FAST_OBJECT(ImageClassifier)
	public:
	private:
		ImageClassifier();
		void execute();

};

}

#endif
