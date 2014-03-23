#include "ImageImporter2D.hpp"
#include <QImage>
#include "DataTypes.hpp"
#include "DeviceManager.hpp"
#include "Exception.hpp"
using namespace fast;

void ImageImporter2D::execute() {
    // TODO check that all parameters needed are present

    // Load image from disk using Qt
    QImage image;
    std::cout << "Trying to load image..." << std::endl;
    if(!image.load(mFilename.c_str())) {
        throw FileNotFoundException(mFilename);
    }
    std::cout << "Loaded image with size " << image.width() << " "  << image.height() << std::endl;

    // Convert image to make sure color tables are not used
    QImage convertedImage = image.convertToFormat(QImage::Format_RGB32);

    // Get pixel data
    const unsigned char * pixelData = convertedImage.constBits();
    // The pixel data array should contain one uchar value for the
    // R, G, B, A components for each pixel

    // TODO: do some conversion to requested output format, also color vs. no color
    float * convertedPixelData = new float[image.width()*image.height()];
    for(int i = 0; i < image.width()*image.height(); i++) {
        // Converted to grayscale
        convertedPixelData[i] = (1.0f/255.0f)*(float)(pixelData[i*4]+pixelData[i*4+1]+pixelData[i*4+2])/3.0f;
    }

    // Transfer to texture(if OpenCL) or copy raw pixel data (if host)
    if(mDevice->isHost()) {
    } else {
        OpenCLDevice::Ptr device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
        cl::Image2D* clImage = new cl::Image2D(
                device->getContext(),
                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                cl::ImageFormat(CL_R, CL_FLOAT),
                image.width(), image.height(),
                0,
                convertedPixelData
        );
        delete[] convertedPixelData;
        mOutput2.lock()->setOpenCLImage(clImage, device);
    }
}

ImageImporter2D::ImageImporter2D() {
    mOutput = Image2D::New();
    mOutput2 = mOutput;
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}

Image2D::Ptr ImageImporter2D::getOutput() {
    if(mOutput != NULL) {
        mOutput->addParent(mPtr.lock());

        Image2D::Ptr newSmartPtr;
        newSmartPtr.swap(mOutput);

        return newSmartPtr;
    } else {
        return mOutput2.lock();
    }
}

void ImageImporter2D::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

void ImageImporter2D::setDevice(ExecutionDevice::Ptr device) {
    mDevice = device;
    mIsModified = true;
}
