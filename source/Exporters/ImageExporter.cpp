#include "ImageExporter.hpp"
#include <QImage>
#include "Exception.hpp"
#include "Utility.hpp"
#include "Image.hpp"

namespace fast {

void ImageExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

ImageExporter::ImageExporter() {
    mFilename = "";
    mIsModified = true;
}

void ImageExporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to ImageExporter");

    Image::pointer input = getStaticInputData<Image>();

    if(input->getDimensions() != 2)
        throw Exception("Input image to ImageExporter must be 2D.");

    QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

    // TODO have to do some type conversion here, assuming float for now
    unsigned char * pixelData = image.bits();
    ImageAccess::pointer access = input->getImageAccess(ACCESS_READ);
    void * inputData = access->get();

    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        float data;
        switch(input->getDataType()) {
        case TYPE_FLOAT:
            data = round(((float*)inputData)[i]*255.0f);
            break;
        case TYPE_UINT8:
            data = ((uchar*)inputData)[i];
            break;
        case TYPE_INT8:
            data = ((char*)inputData)[i]+128;
            break;
        case TYPE_UINT16:
            data = ((ushort*)inputData)[i];
            break;
        case TYPE_INT16:
            data = ((short*)inputData)[i];
            break;

        }
        pixelData[i*4] = data;
        pixelData[i*4+1] = pixelData[i*4];
        pixelData[i*4+2] = pixelData[i*4];
        pixelData[i*4+3] = 255; // Alpha
    }

    image.save(QString(mFilename.c_str()));

}

}; // end namespace fast
