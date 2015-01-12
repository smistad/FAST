#include "ImageFileImporter.hpp"
#include "MetaImageImporter.hpp"
#include "ImageImporter.hpp"
#include <algorithm>

namespace fast {

void ImageFileImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

ImageFileImporter::ImageFileImporter() {
    mFilename = "";
}

Image::pointer ImageFileImporter::getOutput() {
    return getOutputData<Image>(0);
}

inline bool matchExtension(std::string extension, std::string extension2) {
    // Convert to lower case first
    std::transform(extension2.begin(), extension2.end(), extension2.begin(), ::tolower);
    return extension == extension2;

}

void ImageFileImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename was given to the ImageFileImporter");

    // Get file extension
    std::string ext = mFilename.substr(mFilename.rfind(".")+1);
    if(matchExtension(ext, "mhd")) {
        MetaImageImporter::pointer importer = MetaImageImporter::New();
        importer->setFilename(mFilename);
        // Manually set the output data of this importer
        //importer->setOutputData(0, getOutputData<Image>(0));
        setInputConnection(importer->getOutputPort());
        importer->update();
        Image::pointer data = getInputData(0);
        setOutputData(0, data);
    } else if(matchExtension(ext, "jpg") ||
            matchExtension(ext, "jpeg") ||
            matchExtension(ext, "png") ||
            matchExtension(ext, "bmp")) {
        ImageImporter::pointer importer = ImageImporter::New();
        importer->setFilename(mFilename);
        // Manually set the output data of this importer
        setInputConnection(importer->getOutputPort());
        importer->update(); // Have to to update because otherwise getInputData will not be available
        std::cout << "Image importer finished" << std::endl;
        // Set input to be output
        Image::pointer data = getInputData(0);
        std::cout << "Got input data" << std::endl;
        setOutputData(0, data);
        std::cout << "Set output data" << std::endl;
        //importer->setOutputData(0, getOutputData<Image>(0));
        //importer->update();
    } else {
        throw Exception("The ImageFileImporter does not recognize the file extension " + ext);
    }

}

}
