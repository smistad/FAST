#include "ImageImporter.hpp"
#include <memory>
#include <QImage>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <cctype>
#include <algorithm>

namespace fast {

void ImageImporter::execute() {
    if (mFilename == "")
        throw Exception("No filename was supplied to the ImageImporter");

    uchar* convertedPixelData;
    // Load image from disk using Qt
    QImage image;
    reportInfo() << "Trying to load image..." << Reporter::end();
    if(!image.load(mFilename.c_str())) {
        throw FileNotFoundException(mFilename);
    }
    reportInfo() << "Loaded image with size " << image.width() << " "  << image.height() << Reporter::end();

    QImage::Format format;
    if(mGrayscale) {
        format = QImage::Format_Grayscale8;
    } else {
        format = QImage::Format_RGB888;
    }
    QImage convertedImage = image.convertToFormat(format);

    // Get pixel data
    convertedPixelData = convertedImage.bits();

    Image::pointer output = getOutputData<Image>();
    if(convertedImage.width()*convertedImage.depth()/8 != convertedImage.bytesPerLine()) {
        const int bytesPerPixel = (convertedImage.depth()/8);
        std::unique_ptr<uchar[]> fixedPixelData = std::make_unique<uchar[]>(image.width()*image.height()*bytesPerPixel);
        // Misalignment
        for(int scanline = 0; scanline < image.height(); ++scanline) {
            std::memcpy(
                    &fixedPixelData[scanline*image.width()*bytesPerPixel],
                    &convertedPixelData[scanline*convertedImage.bytesPerLine()],
                    image.width()*bytesPerPixel
            );
        }
        output->create(
            image.width(),
            image.height(),
            TYPE_UINT8,
            mGrayscale ? 1 : 3,
            getMainDevice(),
            fixedPixelData.get()
        );
    } else {
        output->create(
            image.width(),
            image.height(),
            TYPE_UINT8,
            mGrayscale ? 1 : 3,
            getMainDevice(),
            convertedPixelData
        );
    }
}

ImageImporter::ImageImporter() {
    mFilename = "";
    mIsModified = true;
    mGrayscale = true;
    createOutputPort<Image>(0);
}

void ImageImporter::setGrayscale(bool grayscale) {
    mGrayscale = grayscale;
}

void ImageImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

}
