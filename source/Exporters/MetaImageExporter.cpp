#include "MetaImageExporter.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
#include <fstream>
using namespace fast;

void MetaImageExporter::setInput(ImageData::pointer input) {
    mInput = input;
    setParent(input);
    mIsModified = true;
}

void MetaImageExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

MetaImageExporter::MetaImageExporter() {
    mFilename = "";
    mIsModified = true;
}

template <class T>
void writeToRawFile(std::string filename, T * data, unsigned int numberOfElements) {
    // TODO use mapped_file_sink form boost instead
    FILE* file = fopen(filename.c_str(), "wb");
    if(file == NULL) {
        throw Exception("Could not open file " + filename + " for writing");
    }

    fwrite(data, sizeof(T), numberOfElements, file);

    fclose(file);
}

void MetaImageExporter::execute() {

    if(!mInput.isValid())
        throw Exception("No input was given to the MetaImageExporter");

    if(mFilename == "")
        throw Exception("No filename was given to the MetaImageExporter");

    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    // Check that filename ends with .mhd if not, append it
    if(mFilename.substr(mFilename.length()-4) != ".mhd") {
        mFilename += ".mhd";
    }

    // Create mhd file
    std::fstream mhdFile;
    mhdFile.open(mFilename.c_str(), std::fstream::out);
    if(!mhdFile.is_open()) {
        throw Exception("Could not open file " + mFilename + " for writing");
    }

    mhdFile << "ObjectType = Image\n";
    if(input->getDimensions() == 2) {
        mhdFile << "NDims = 2\n";
        mhdFile << "DimSize = " << input->getWidth() << " " <<
                input->getHeight() << "\n";
    } else {
        mhdFile << "NDims = 3\n";
        mhdFile << "DimSize = " << input->getWidth() << " " <<
            input->getHeight() << " " << input->getDepth() << "\n";
    }
    mhdFile << "BinaryData = True\n";
    mhdFile << "CompressedData = False\n";
    mhdFile << "ElementNumberOfChannels = " << input->getNrOfComponents() << "\n";
    mhdFile << "ElementSpacing = " << input->getSpacing()[0] << " " << input->getSpacing()[1];
    if(input->getDimensions() == 3)
        mhdFile << " " << input->getSpacing()[2];
    mhdFile << "\n";
    mhdFile << "Offset = " << input->getOffset()[0] << " " << input->getOffset()[1] << " " << input->getOffset()[2] << "\n";
    mhdFile << "CenterOfRotation = " << input->getCenterOfRotation()[0] << " " << input->getCenterOfRotation()[1] << " " << input->getCenterOfRotation()[2] << "\n";
    mhdFile << "TransformMatrix =";
    for(unsigned int i = 0; i < 9; i++)
        mhdFile << " " << input->getTransformMatrix()[i];
    mhdFile << "\n";

    // Save to raw file
    // set rawFilename, by removing the end .mhd from mFilename and add .raw
    std::string rawFilename = mFilename.substr(0,mFilename.length()-4) + ".raw";
    const unsigned int numberOfElements = input->getWidth()*input->getHeight()*
            input->getDepth()*input->getNrOfComponents();

    ImageAccess access = input->getImageAccess(ACCESS_READ);
    void* data = access.get();
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        mhdFile << "ElementType = MET_FLOAT\n";
        writeToRawFile<float>(rawFilename,(float*)data,numberOfElements);
        break;
    case TYPE_UINT8:
        mhdFile << "ElementType = MET_UCHAR\n";
        writeToRawFile<uchar>(rawFilename,(uchar*)data,numberOfElements);
        break;
    case TYPE_INT8:
        mhdFile << "ElementType = MET_CHAR\n";
        writeToRawFile<char>(rawFilename,(char*)data,numberOfElements);
        break;
    case TYPE_UINT16:
        mhdFile << "ElementType = MET_USHORT\n";
        writeToRawFile<ushort>(rawFilename,(ushort*)data,numberOfElements);
        break;
    case TYPE_INT16:
        mhdFile << "ElementType = MET_SHORT\n";
        writeToRawFile<short>(rawFilename,(short*)data,numberOfElements);
        break;
    }

    mhdFile << "ElementDataFile = " << rawFilename << "\n";

    mhdFile.close();
}
