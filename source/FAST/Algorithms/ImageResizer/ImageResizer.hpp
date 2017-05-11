#ifndef IMAGE_RESIZER_HPP
#define IMAGE_RESIZER_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageResizer : public ProcessObject {
	FAST_OBJECT(ImageResizer)
	public:
		void setWidth(int width);
		void setHeight(int height);
		void setDepth(int depth);
		void setSize(VectorXi size);
	private:
		ImageResizer();
		void execute();

		Vector3i mSize;
};

}

#endif
