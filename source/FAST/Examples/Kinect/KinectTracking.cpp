#include "KinectTracking.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

KinectTracking::KinectTracking() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_STATIC);

    // Create annotation image
    mAnnotationImage = Image::New();
    mAnnotationImage->create(512, 424, TYPE_UINT8, 1);
    mAnnotationImage->fill(0);
}

void KinectTracking::execute() {
    Image::pointer input = getStaticInputData<Image>();

    setStaticOutputData<Image>(0, input);
    setStaticOutputData<Image>(1, mAnnotationImage);
}

void KinectTracking::addLine(Vector2i start, Vector2i end) {
    std::cout << "Drawing from: " << start.transpose() << " to " << end.transpose() << std::endl;
    // Draw line in some auxillary image
    ImageAccess::pointer access = mAnnotationImage->getImageAccess(ACCESS_READ_WRITE);
    Vector2f direction = end.cast<float>() - start.cast<float>();
    int length = (end-start).norm();
    int brushSize = 6;
    for(int i = 0; i < length; ++i) {
        float distance = (float)i/length;
        for(int a = -brushSize; a <= brushSize; a++) {
            for(int b = -brushSize; b <= brushSize; b++) {
                Vector2f offset(a, b);
                if(offset.norm() > brushSize)
                    continue;
                Vector2f position = start.cast<float>() + direction*distance + offset;
                try {
                    access->setScalar(position.cast<int>(), 1);
                } catch(Exception &e) {

                }
            }
        }


    }
}

}