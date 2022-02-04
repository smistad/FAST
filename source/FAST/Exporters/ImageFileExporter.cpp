#include "ImageFileExporter.hpp"
#include "MetaImageExporter.hpp"
#ifdef FAST_MODULE_VISUALIZATION
#include "ImageExporter.hpp"
#endif
#include "FAST/Data/Image.hpp"
#include <algorithm>
#include <utility>

namespace fast {

ImageFileExporter::ImageFileExporter() : ImageFileExporter("") {
}

ImageFileExporter::ImageFileExporter(std::string filename, bool compress, bool resample) : FileExporter(filename) {
    createInputPort<Image>(0);
    setCompression(compress);
    setResampleIfNeeded(resample);
}

inline bool matchExtension(std::string extension, std::string extension2) {
    // Convert to lower case first
    std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
    return extension == extension2;

}

void ImageFileExporter::execute() {
    if(m_filename.empty())
        throw Exception("No filename was given to the ImageFileExporter");

    std::cout << "Getting data.." << std::endl;
    auto input = getInputData<Image>();
    std::cout << "Done" << std::endl;

    // Get file extension
    size_t pos = m_filename.rfind(".", -5);
    if(pos == std::string::npos) {
        throw Exception("ImageFileExporter filename had no extension");
    } else {
        std::string ext = m_filename.substr(pos + 1);
        if(matchExtension(ext, "mhd")) {
            auto exporter = MetaImageExporter::create(m_filename, mCompress)
                    ->connect(input);
            exporter->run();
        } else if(matchExtension(ext, "jpg") ||
                  matchExtension(ext, "jpeg") ||
                  matchExtension(ext, "png") ||
                  matchExtension(ext, "bmp")) {
#ifdef FAST_MODULE_VISUALIZATION
            auto exporter = ImageExporter::create(m_filename, m_resample)
                    ->connect(input);
            exporter->setFilename(m_filename);
            exporter->setInputData(input);
            exporter->update();
#else
            throw Exception("Image export to common image formats such as jpg, png and bmp needs FAST built with Qt");
#endif
        } else {
            throw Exception("The ImageFileExporter does not recognize the file extension " + ext);
        }
    }

}

void ImageFileExporter::setCompression(bool compress) {
    mCompress = compress;
}

void ImageFileExporter::setResampleIfNeeded(bool resample) {
    m_resample = resample;
}

}
