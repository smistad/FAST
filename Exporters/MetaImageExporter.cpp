#include "MetaImageExporter.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
using namespace fast;

void MetaImageExporter::setInput(ImageData::pointer input) {
    mInput = input;
    mIsModified = true;
}

void MetaImageExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

template <class T>
void writeToRawFile(std::string filename, T * data, unsigned int numberOfElements) {
    boost::iostreams::mapped_file_sink file;
    file.open(filename, numberOfElements*sizeof(T));
    if(!file.is_open()) {
        throw Exception("Could not open file " + filename + " for writing");
    }

    T* fileData = (T*)file.data();
    memcpy(fileData,data,numberOfElements*sizeof(T));

    file.close();
}

void MetaImageExporter::execute() {

    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    // Create mhd file
    std::fstream mhdFile;
    mhdFile.open(mFilename.c_str(), std::fstream::out);
    if(!mhdFile.is_open()) {
        throw Exception("Could not open file " + mFilename + " for writing");
    }

    mhdFile.close();

    // Save to raw file
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        break;
    case TYPE_UINT8:
        break;
    case TYPE_INT8:
        break;
    case TYPE_UINT16:
        break;
    case TYPE_INT16:
        break;
    }

}
