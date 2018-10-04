#include "ImageFileExporter.hpp"
#include "MetaImageExporter.hpp"
#include "ImageExporter.hpp"
#include "FAST/Data/Image.hpp"
#include <algorithm>

namespace fast {

void ImageFileExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

ImageFileExporter::ImageFileExporter() {
    mFilename = "";
    createInputPort<Image>(0);
}

inline bool matchExtension(std::string extension, std::string extension2) {
    // Convert to lower case first
    std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
    return extension == extension2;

}

void ImageFileExporter::execute() {
    if(mFilename == "")
        throw Exception("No filename was given to the ImageFileExporter");

    Image::pointer input = getInputData<Image>();

    // Get file extension
    size_t pos = mFilename.rfind(".", -5);
    if(pos == std::string::npos) {
        throw Exception("ImageFileExporter filename had no extension");
    } else {
        std::string ext = mFilename.substr(pos + 1);
        if(matchExtension(ext, "mhd")) {
            MetaImageExporter::pointer exporter = MetaImageExporter::New();
            exporter->setInputData(input);
            exporter->setFilename(mFilename);
            exporter->setCompression(mCompress);
            exporter->update(0);
        } else if(matchExtension(ext, "jpg") ||
                  matchExtension(ext, "jpeg") ||
                  matchExtension(ext, "png") ||
                  matchExtension(ext, "bmp")) {
#ifdef FAST_MODULE_VISUALIZATION
            ImageExporter::pointer exporter = ImageExporter::New();
            exporter->setFilename(mFilename);
            exporter->setInputData(input);
            exporter->update(0);
#else
            throw Exception("The ImageFileExporter needs the visualization module (Qt) to be enabled in order to write image files.");
#endif
        } else {
            throw Exception("The ImageFileExporter does not recognize the file extension " + ext);
        }
    }

}

void ImageFileExporter::setCompression(bool compress) {
    mCompress = compress;
}

}
