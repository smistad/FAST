#include "ImageExporter2D.hpp"
#include <QImage>
using namespace fast;

void ImageExporter2D::setInput(Image2D::Ptr image) {
    mStaticInput = image;
}

void ImageExporter2D::setInput(Image2Dt::Ptr image) {
    mDynamicInput = image;
}

void ImageExporter2D::setFilename(std::string filename) {
    mFilename = filename;
}

ImageExporter2D::ImageExporter2D() {
    mFilename = "";
}

void ImageExporter2D::execute() {
    if(mStaticInput == NULL)
        throw Exception("No input image given to ImageExporter2D");

    if(mFilename == "")
        throw Exception("No filename given to ImageExporter2D");

    Image2D::Ptr input = mStaticInput;

    QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

    // TODO have to do some type conversion here, assuming float for now
    unsigned char * pixelData = image.bits();
    ImageAccess2D access = input->getImageAccess(ACCESS_READ);
    float * inputData = (float *)access.get();

    for(int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        pixelData[i*4] = 255;
        pixelData[i*4+1] = std::round(inputData[i]*255.0f);
        pixelData[i*4+2] = pixelData[i*4+1];
        pixelData[i*4+3] = pixelData[i*4+1];
    }

    image.save(mFilename);

}
