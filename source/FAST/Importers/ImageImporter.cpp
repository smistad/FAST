#include "ImageImporter.hpp"
#ifdef FAST_MODULE_VISUALIZATION
#include <QImage>
#endif
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

void ImageImporter::execute() {
#ifdef FAST_MODULE_VISUALIZATION
    if(mFilename == "")
        throw Exception("No filename was supplied to the ImageImporter");

    // Load image from disk using Qt
    QImage image;
    reportInfo() << "Trying to load image..." << Reporter::end;
    if(!image.load(mFilename.c_str())) {
        throw FileNotFoundException(mFilename);
    }
    reportInfo() << "Loaded image with size " << image.width() << " "  << image.height() << Reporter::end;

    // Convert image to make sure color tables are not used
    QImage convertedImage = image.convertToFormat(QImage::Format_RGB32);

    // Get pixel data
    const unsigned char * pixelData = convertedImage.constBits();
    // The pixel data array should contain one uchar value for the
    // R, G, B, A components for each pixel

    // TODO: do some conversion to requested output format, also color vs. no color
    uchar* convertedPixelData = new uchar[image.width()*image.height()];
    for(int i = 0; i < image.width()*image.height(); i++) {
        // Converted to grayscale
        convertedPixelData[i] = (uchar)round((pixelData[i*4]+pixelData[i*4+1]+pixelData[i*4+2])/3.0f);
    }

    // Transfer to texture(if OpenCL) or copy raw pixel data (if host)
    Image::pointer output = getOutputData<Image>();
    output->create(
            image.width(),
            image.height(),
            TYPE_UINT8,
            1,
            getMainDevice(),
            convertedPixelData
    );
    delete[] convertedPixelData;
#else
    throw Exception("The ImageImporter need Qt to work, but the visualization module is disabled");
#endif
}

ImageImporter::ImageImporter() {
	mFilename = "";
	mIsModified = true;
    createOutputPort<Image>(0, OUTPUT_STATIC);
}

void ImageImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}
