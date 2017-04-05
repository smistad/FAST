#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Data/Image.hpp"
#ifdef FAST_MODULE_VISUALIZATION
#include <QImage>
#endif

namespace fast {

void ImageExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

ImageExporter::ImageExporter() {
    createInputPort<Image>(0);
    mFilename = "";
    mIsModified = true;
}

void ImageExporter::execute() {
#ifdef FAST_MODULE_VISUALIZATION
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
    uint nrOfComponents = input->getNrOfComponents();

    for(uint x = 0; x < input->getWidth(); x++) {
    for(uint y = 0; y < input->getHeight(); y++) {
        float data;
        switch(input->getDataType()) {
        case TYPE_FLOAT:
            data = round(((float*)inputData)[(x+y*input->getWidth())*nrOfComponents]*255.0f);
            break;
        case TYPE_UINT8:
            data = ((uchar*)inputData)[(x+y*input->getWidth())*nrOfComponents];
            break;
        case TYPE_INT8:
            data = ((char*)inputData)[(x+y*input->getWidth())*nrOfComponents]+128;
            break;
        case TYPE_UINT16:
            data = ((ushort*)inputData)[(x+y*input->getWidth())*nrOfComponents];
            break;
        case TYPE_INT16:
            data = ((short*)inputData)[(x+y*input->getWidth())*nrOfComponents];
            break;

        }
        uint i = x + y*input->getWidth();
        pixelData[i*4] = data;
        pixelData[i*4+1] = pixelData[i*4];
        pixelData[i*4+2] = pixelData[i*4];
        pixelData[i*4+3] = 255; // Alpha
    }}

    image.save(QString(mFilename.c_str()));

#else
    throw Exception("The ImageExporter need Qt to work, but the visualization module is disabled");
#endif
}

}; // end namespace fast
