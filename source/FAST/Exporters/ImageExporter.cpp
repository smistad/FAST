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

    Image::pointer input = getInputData<Image>();

    if(input->getDimensions() != 2)
        throw Exception("Input image to ImageExporter must be 2D.");


    QImage::Format format = QImage::Format_RGB32;
    if(input->getNrOfChannels() == 1) {
        format = QImage::Format_Grayscale8;
    }
    QImage image(input->getWidth(), input->getHeight(), format);

    unsigned char * pixelData = image.bits();
    auto access = input->getImageAccess(ACCESS_READ);
    void * inputData = access->get();
    if(input->getNrOfChannels() == 1 && input->getDataType() == TYPE_UINT8) {
        // Fast simple copy for single channel/grayscale uchar copy only
        std::memcpy(pixelData, inputData, sizeof(uchar)*input->getWidth()*input->getHeight());
    } else {
        // More complicated copy which supports more..
        const uint nrOfChannels = input->getNrOfChannels();
        for (uint y = 0; y < input->getHeight(); ++y) {
            for (uint x = 0; x < input->getWidth(); ++x) {
                std::vector<uchar> channelData;
                for (uint channel = 0; channel < nrOfChannels; ++channel) {
                    uchar data = 0;
                    switch (input->getDataType()) {
                        case TYPE_FLOAT:
                            data = round(((float *) inputData)[(x + y * input->getWidth()) * nrOfChannels + channel] *
                                         255.0f);
                            break;
                        case TYPE_UINT8:
                            data = ((uchar *) inputData)[(x + y * input->getWidth()) * nrOfChannels + channel];
                            break;
                        case TYPE_INT8:
                            data = ((char *) inputData)[(x + y * input->getWidth()) * nrOfChannels + channel] + 128;
                            break;
                        case TYPE_UINT16:
                            data = ((ushort *) inputData)[(x + y * input->getWidth()) * nrOfChannels + channel];
                            break;
                        case TYPE_INT16:
                            data = ((short *) inputData)[(x + y * input->getWidth()) * nrOfChannels + channel];
                            break;
                    }
                    channelData.push_back(data);
                }
                uint i = x + y * input->getWidth();
                pixelData[i * 4 + 0] = channelData[0];
                pixelData[i * 4 + 1] = channelData[1 % nrOfChannels];
                pixelData[i * 4 + 2] = channelData[2 % nrOfChannels];
                pixelData[i * 4 + 3] = 255; // Alpha
            }
        }
    }

    image.save(QString(mFilename.c_str()));

#else
    throw Exception("The ImageExporter need Qt to work, but the visualization module is disabled");
#endif
}

}; // end namespace fast
