#include "MetaImageExporter.hpp"
#include "FAST/Data/Image.hpp"
#include <fstream>
#ifdef ZLIB_ENABLED
#include <zlib.h>
#endif

namespace fast {

void MetaImageExporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

MetaImageExporter::MetaImageExporter() {
    createInputPort<Image>(0);
    mFilename = "";
    mIsModified = true;
    mUseCompression = false;
}

template <class T>
inline std::size_t writeToRawFile(std::string filename, T * data, unsigned int numberOfElements, bool useCompression) {
    // TODO use mapped_file_sink form boost instead
    FILE* file = fopen(filename.c_str(), "wb");
    if(file == NULL) {
        throw Exception("Could not open file " + filename + " for writing");
    }
    std::size_t returnSize;
    if(useCompression) {
#ifdef ZLIB_ENABLED
        // Have to allocate enough memory for compression: 1.1*DATA_SIZE_IN_BYTES + 12
        std::size_t sizeDataCompressed = compressBound(sizeof(T)*numberOfElements);
        std::size_t sizeDataOriginal = sizeof(T)*numberOfElements;
        Bytef* writeData = (Bytef*)malloc(sizeDataCompressed);
        int z_result = compress(
                (Bytef*)writeData,
                (uLongf*)&sizeDataCompressed,
                (Bytef*)data,
                sizeDataOriginal
        );
        switch(z_result) {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            throw Exception("Out of memory while compressing raw file");
            break;
        case Z_BUF_ERROR:
            throw Exception("Output buffer was not large enough while compressing raw file");
            break;
        }
        // sizeDataCompressed was changed after compress call
        fwrite(writeData, sizeDataCompressed, 1, file);
        returnSize = sizeDataCompressed;
        fclose(file);
        free(writeData);
#endif
    } else {
        returnSize = sizeof(T)*numberOfElements;
        fwrite(data, sizeof(T), numberOfElements, file);
        fclose(file);
    }


    return returnSize;
}

void MetaImageExporter::execute() {
#ifndef ZLIB_ENABLED
    if(mUseCompression)
        throw Exception("You enabled compression on the MetaImageExporter, however FAST is not compiled with zlib which is required to do so.");
#endif

    if(mFilename == "")
        throw Exception("No filename was given to the MetaImageExporter");

    Image::pointer input = getStaticInputData<Image>();

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
    mhdFile << "ElementNumberOfChannels = " << input->getNrOfComponents() << "\n";
    mhdFile << "ElementSpacing = " << input->getSpacing()[0] << " " << input->getSpacing()[1];
    if(input->getDimensions() == 3)
        mhdFile << " " << input->getSpacing()[2];
    mhdFile << "\n";
    AffineTransformation T = SceneGraph::getAffineTransformationFromData(input);
    mhdFile << "Offset = " << T.translation().x() << " " << T.translation().y() << " " << T.translation().z() << "\n";
    mhdFile << "TransformMatrix =";

    for(unsigned int i = 0; i < 3; i++) {
    for(unsigned int j = 0; j < 3; j++) {
        mhdFile << " " << T.matrix()(j,i);
    }}
    mhdFile << "\n";

    // Save to raw file
    // set rawFilename, by removing the end .mhd from mFilename and add .raw
    std::string extension = ".raw";
#ifdef ZLIB_ENABLED
    if(mUseCompression) {
        extension = ".zraw";
    }
#endif
    std::string rawFilename = mFilename.substr(0,mFilename.length()-4) + extension;
    const unsigned int numberOfElements = input->getWidth()*input->getHeight()*
            input->getDepth()*input->getNrOfComponents();

    ImageAccess::pointer access = input->getImageAccess(ACCESS_READ);
    void* data = access->get();
    std::size_t compressedSize;
    switch(input->getDataType()) {
    case TYPE_FLOAT:
        mhdFile << "ElementType = MET_FLOAT\n";
        compressedSize = writeToRawFile<float>(rawFilename,(float*)data,numberOfElements,mUseCompression);
        break;
    case TYPE_UINT8:
        mhdFile << "ElementType = MET_UCHAR\n";
        compressedSize = writeToRawFile<uchar>(rawFilename,(uchar*)data,numberOfElements,mUseCompression);
        break;
    case TYPE_INT8:
        mhdFile << "ElementType = MET_CHAR\n";
        compressedSize = writeToRawFile<char>(rawFilename,(char*)data,numberOfElements,mUseCompression);
        break;
    case TYPE_UINT16:
        mhdFile << "ElementType = MET_USHORT\n";
        compressedSize = writeToRawFile<ushort>(rawFilename,(ushort*)data,numberOfElements,mUseCompression);
        break;
    case TYPE_INT16:
        mhdFile << "ElementType = MET_SHORT\n";
        compressedSize = writeToRawFile<short>(rawFilename,(short*)data,numberOfElements,mUseCompression);
        break;
    }

#ifdef ZLIB_ENABLED
    if(mUseCompression) {
        mhdFile << "CompressedData = True" << "\n";
        mhdFile << "CompressedDataSize = " << compressedSize << "\n";
    }
#endif

    // Remove any path information from rawFilename
    std::size_t slashPos = rawFilename.find_last_of('/');
    if(slashPos != std::string::npos) {
        rawFilename = rawFilename.substr(slashPos+1);
    }

    mhdFile << "ElementDataFile = " << rawFilename << "\n";

    mhdFile.close();
}


void MetaImageExporter::enableCompression() {
    mUseCompression = true;
    mIsModified = true;
}

void MetaImageExporter::disableCompression() {
    mUseCompression = false;
    mIsModified = true;
}


}
