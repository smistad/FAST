#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Data/Image.hpp"
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include <QImage>
#include <FAST/Algorithms/Compression/JPEGXLCompression.hpp>
#include <fstream>

namespace fast {


void ImageExporter::loadAttributes() {
    setFilename(getStringAttribute("filename"));
}

ImageExporter::ImageExporter() : ImageExporter("") {
}

ImageExporter::ImageExporter(std::string filename, int quality, bool resample) : FileExporter(filename) {
    createInputPort<Image>(0);
    createStringAttribute("filename", "Filename", "Path to file to load", filename);
    m_resample = resample;
    setQuality(quality);
}

void ImageExporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename given to ImageExporter");

    auto input = getInputData<Image>();

    if(input->getDimensions() != 2)
        throw Exception("Input image to ImageExporter must be 2D.");

    Vector3f spacing = input->getSpacing();
    if(m_resample && spacing.x() != spacing.y()) {
        // Resample image so that it is isotropic
        float targetSpacing = std::min(spacing.x(), spacing.y());
        input = ImageResampler::create(targetSpacing, targetSpacing)
                ->connect(input)
                ->runAndGetOutputData<Image>();
    }

    size_t pos = m_filename.rfind(".", -5);
    if(pos == std::string::npos)
        throw Exception("ImageExporter filename had no extension");

    std::string ext = m_filename.substr(pos + 1);
    if(stringToLower("jxl") == ext) {
        JPEGXLCompression jxl;
        auto access = input->getImageAccess(ACCESS_READ);
        void * inputData = access->get();
        // Compress pixels with JPEG XL and store to binary file
        std::vector<uchar> compressedData;
        jxl.compress(inputData, input->getWidth(), input->getHeight(), &compressedData, m_quality);
        std::ofstream file(m_filename, std::fstream::binary);
        std::copy(compressedData.begin(), compressedData.end(), std::ostreambuf_iterator<char>(file));
        file.close();
    } else {
        auto format = QImage::Format_RGBA8888;
        int Qchannels = 4;
        if(input->getNrOfChannels() == 1) {
            format = QImage::Format_Grayscale8;
            Qchannels = 1;
        }
        QImage image(input->getWidth(), input->getHeight(), format);

        unsigned char * pixelData = image.bits();
        auto access = input->getImageAccess(ACCESS_READ);
        void * inputData = access->get();
        if(input->getNrOfChannels() == 1 && input->getDataType() == TYPE_UINT8) {
            // Fast simple copy for single channel/grayscale uchar copy only
            if(image.width() * image.depth() / 8 != image.bytesPerLine()) { // Misalignment: QImage requires 32-bit alignment per scanline
                // Copy per scan line
                for(int scanline = 0; scanline < image.height(); ++scanline) {
                    std::memcpy(
                             (void*)(&pixelData[scanline * image.bytesPerLine()]),
                             (void*)(&((uchar*)inputData)[scanline * image.width()]),
                            image.width()
                            );
                }
            } else {
                // No misalignment, simply just copy the entire image
                std::memcpy(pixelData, inputData, sizeof(uchar) * input->getWidth() * input->getHeight());
            }
        } else {
            // More complicated copy which supports more..
            const uint nrOfChannels = input->getNrOfChannels();
            for(uint y = 0; y < input->getHeight(); ++y) {
                for(uint x = 0; x < input->getWidth(); ++x) {
                    std::vector<uchar> channelData;
                    for(uint channel = 0; channel < nrOfChannels; ++channel) {
                        uchar data = 0;
                        switch(input->getDataType()) {
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
                    uint i = Qchannels*x + y * image.bytesPerLine();
                    pixelData[i] = channelData[0];
                    if(Qchannels == 4) {
                        pixelData[i + 1] = channelData[1 % nrOfChannels];
                        pixelData[i + 2] = channelData[2 % nrOfChannels];
                        pixelData[i + 3] = 255; // Alpha
                    }
                }
            }
        }

        image.save(QString(m_filename.c_str()), nullptr, m_quality);
    }
}

void ImageExporter::setResampleIfNeeded(bool resample) {
    m_resample = resample;
}

void ImageExporter::setQuality(int quality) {
    if(quality < 0 || quality > 100)
        throw Exception("Quality in ImageExporter must be between 0 and 100");
    m_quality = quality;
}

}; // end namespace fast
