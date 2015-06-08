#include "MetaImageImporter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#ifdef ZLIB_ENABLED
#include <zlib.h>
#endif
using namespace fast;

void MetaImageImporter::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

MetaImageImporter::MetaImageImporter() {
    mFilename = "";
    mIsModified = true;
    createOutputPort<Image>(0, OUTPUT_STATIC);
}

std::vector<std::string> stringSplit(std::string str, std::string delimiter) {
    std::vector<std::string> values;
    int startPosition = 0;
    int endPosition = str.find(delimiter, startPosition+1);
    while(true) {
        if(endPosition == std::string::npos) {
            endPosition = str.size();
            values.push_back(str.substr(startPosition, endPosition-startPosition));
            break;
        } else {
            values.push_back(str.substr(startPosition, endPosition-startPosition));
            startPosition = endPosition+1;
            endPosition = str.find(delimiter, startPosition+1);
        }
    }
    return values;
}

template <class T>
inline void * readRawData(std::string rawFilename, unsigned int width, unsigned int height, unsigned int depth, unsigned int nrOfComponents, bool compressed, std::size_t compressedFileSize) {
    T * data = new T[width*height*depth*nrOfComponents];
    if(compressed) {
#ifdef ZLIB_ENABLED
        // Read compressed data
        boost::iostreams::mapped_file_source file;
        file.open(rawFilename, width*height*depth*nrOfComponents*sizeof(T));
        if(!file.is_open())
            throw FileNotFoundException(rawFilename);
        Bytef* fileData = (Bytef*)file.data();

        unsigned long uncompressedSize = sizeof(T)*width*height*depth*nrOfComponents;
        unsigned long fileSize = file.size();
		int z_result = uncompress(
            (Bytef*)data,       // destination for the uncompressed
                                    // data.  This should be the size of
                                    // the original data, which you should
                                    // already know.

            (uLongf *)&uncompressedSize,  // length of destination (uncompressed)
                                    // buffer

            (Bytef*)fileData,   // source buffer - the compressed data

            (uLong)compressedFileSize);   // length of compressed data in bytes
        switch( z_result )
        {
        case Z_OK:
            break;

        case Z_MEM_ERROR:
            throw Exception("Out of memory while decompressing raw file");
            break;

        case Z_BUF_ERROR:
            throw Exception("Output buffer was not large enough while decompressing raw file");
            break;
        }
        file.close();
#else
        throw Exception("Error reading MetaImage. Compressed raw files (.zraw) currently not supported.");
#endif
    } else {
        boost::iostreams::mapped_file_source file;
        file.open(rawFilename, width*height*depth*nrOfComponents*sizeof(T));
        if(!file.is_open())
            throw FileNotFoundException(rawFilename);
        T * fileData = (T*)file.data();
        memcpy(data,fileData,width*height*depth*nrOfComponents*sizeof(T));
        file.close();
    }
    return data;
}

void MetaImageImporter::execute() {
    if(mFilename == "")
        throw Exception("Filename was not set in MetaImageImporter");

    // Open and parse mhd file
    std::fstream mhdFile;
    mhdFile.open(mFilename.c_str(), std::fstream::in);
    if(!mhdFile.is_open())
        throw FileNotFoundException(mFilename);
    std::string line;
    std::string rawFilename;
    bool sizeFound = false,
         rawFilenameFound = false,
         typeFound = false,
         dimensionsFound = false;
    std::string typeName;

    // Find NDims first
    bool imageIs3D = false;
    do {
        std::getline(mhdFile, line);
        if(line.substr(0, 5) == "NDims") {
            if(line.substr(5+3, 1) == "3") {
                imageIs3D = true;
            } else if(line.substr(5+3, 1) == "2") {
                imageIs3D = false;
            }
            dimensionsFound = true;
        }
    } while(!mhdFile.eof() && !dimensionsFound);

    if(!dimensionsFound)
        throw Exception("NDims not found in metaimage file.");

    // Reset and start reading file from beginning
    mhdFile.seekg(0);

    unsigned int width, height, depth = 1;
    unsigned int nrOfComponents = 1;
    Image::pointer output = getOutputData<Image>(0);

    Vector3f spacing(1,1,1), offset(0,0,0), centerOfRotation(0,0,0);
    Matrix3f transformMatrix = Matrix3f::Identity();
    bool isCompressed = false;
    std::size_t compressedDataSize = 0;

    do{
        std::getline(mhdFile, line);
        boost::trim(line);
        if(line.size() == 0) // line is empty
            continue;
        int firstSpace = line.find(" ");
        std::string key = line.substr(0, firstSpace);
        boost::trim(key);
        int equalSignPos = line.find("=");
        std::string value = line.substr(equalSignPos+1);
        boost::trim(value);
        if(key == "DimSize") {
            std::vector<std::string> values;
            boost::split(values, value, boost::is_any_of(" "));
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());

            if(imageIs3D) {
                if(values.size() != 3)
                    throw Exception("DimSize in MetaImage file did not contain 3 numbers");
                depth = boost::lexical_cast<int>(values[2]);
            } else {
                if(values.size() != 2)
                    throw Exception("DimSize in MetaImage file did not contain 2 numbers");
            }
            width = boost::lexical_cast<int>(values[0]);
            height = boost::lexical_cast<int>(values[1]);
            sizeFound = true;
        } else if(key == "CompressedData" && value == "True") {
            isCompressed = true;
        } else if(key == "CompressedDataSize") {
            compressedDataSize = boost::lexical_cast<int>(value);
        } else if(key == "ElementDataFile") {
            rawFilename = value;
            rawFilenameFound = true;

            // Remove any trailing spaces
            int pos = rawFilename.find(" ");
            if(pos > 0)
            rawFilename = rawFilename.substr(0,pos);

            // Get path name
            pos = mFilename.rfind('/');
            if(pos > 0)
                rawFilename = mFilename.substr(0,pos+1) + rawFilename;
        } else if(key == "ElementType") {
            typeFound = true;
            typeName = value;

            // Remove any trailing spaces
            int pos = typeName.find(" ");
            if(pos > 0)
            typeName = typeName.substr(0,pos);

            if(typeName == "MET_SHORT") {
            } else if(typeName == "MET_USHORT") {
            } else if(typeName == "MET_CHAR") {
            } else if(typeName == "MET_UCHAR") {
            } else if(typeName == "MET_INT") {
            } else if(typeName == "MET_UINT") {
            } else if(typeName == "MET_FLOAT") {
            } else {
                throw Exception("Trying to read volume of unsupported data type", __LINE__, __FILE__);
            }
        } else if(key == "ElementNumberOfChannels") {
            nrOfComponents = boost::lexical_cast<int>(value.c_str());
            if(nrOfComponents <= 0)
                throw Exception("Error in reading the number of components in the MetaImageImporter");
        } else if(key == "ElementSpacing") {
            std::vector<std::string> values;
            boost::split(values, value, boost::is_any_of(" "));
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            if(imageIs3D) {
                if(values.size() != 3)
                    throw Exception("ElementSpacing in MetaImage file did not contain 3 numbers");
                spacing[0] = boost::lexical_cast<float>(values[0]);
                spacing[1] = boost::lexical_cast<float>(values[1]);
                spacing[2] = boost::lexical_cast<float>(values[2]);
            } else {
                if(values.size() != 2 && values.size() != 3)
                    throw Exception("ElementSpacing in MetaImage file did not contain 2 or 3 numbers");

                spacing[0] = boost::lexical_cast<float>(values[0]);
                spacing[1] = boost::lexical_cast<float>(values[1]);
                if(values.size() == 2) {
                    spacing[2] = 1;
                } else {
                    spacing[2] = boost::lexical_cast<float>(values[2]);
                }
            }


        } else if(key == "CenterOfRotation") {
            //std::cout << "WARNING: CenterOfRotation in Metaimage file ignored" << std::endl;
            std::vector<std::string> values;
            boost::split(values, value, boost::is_any_of(" "));
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            if(imageIs3D) {
                if(values.size() != 3)
                    throw Exception("CenterOfRotation in MetaImage file did not contain 3 numbers");
                centerOfRotation[0] = boost::lexical_cast<float>(values[0]);
                centerOfRotation[1] = boost::lexical_cast<float>(values[1]);
                centerOfRotation[2] = boost::lexical_cast<float>(values[2]);
            } else {
                if(values.size() != 2 && values.size() != 3)
                    throw Exception("CenterOfRotation in MetaImage file did not contain 2 or 3 numbers");

                centerOfRotation[0] = boost::lexical_cast<float>(values[0]);
                centerOfRotation[1] = boost::lexical_cast<float>(values[1]);
                if(values.size() == 2) {
                    centerOfRotation[2] = 0;
                } else {
                    centerOfRotation[2] = boost::lexical_cast<float>(values[2]);
                }
            }
        } else if(key == "Offset" || key == "Origin" || key == "Position") {
            std::vector<std::string> values;
            boost::split(values, value, boost::is_any_of(" "));
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            if(values.size() != 3)
                throw Exception("Offset/Origin/Position in MetaImage file did not contain 3 numbers");

            offset[0] = boost::lexical_cast<float>(values[0].c_str());
            offset[1] = boost::lexical_cast<float>(values[1].c_str());
            offset[2] = boost::lexical_cast<float>(values[2].c_str());
        } else if(key == "TransformMatrix" || key == "Rotation" || key == "Orientation") {
            std::vector<std::string> values;
            boost::split(values, value, boost::is_any_of(" "));
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            if(values.size() != 9)
                throw Exception("Encountered a transform/orientation/rotation matrix with incorrect number of elements in the MetaImageImporter");

            for(unsigned int i = 0; i < 3; i++) {
            for(unsigned int j = 0; j < 3; j++) {
                transformMatrix(j,i) = boost::lexical_cast<float>(values[j+i*3].c_str());
            }}
        }

    } while(!mhdFile.eof());

    mhdFile.close();
    if(!sizeFound || !rawFilenameFound || !typeFound || !dimensionsFound)
        throw Exception("Error reading the mhd file", __LINE__, __FILE__);


    void * data;
    DataType type;
    if(typeName == "MET_SHORT") {
        type = TYPE_INT16;
        data = readRawData<short>(rawFilename, width, height, depth, nrOfComponents, isCompressed, compressedDataSize);
    } else if(typeName == "MET_USHORT") {
        type = TYPE_UINT16;
        data = readRawData<unsigned short>(rawFilename, width, height, depth, nrOfComponents, isCompressed, compressedDataSize);
    } else if(typeName == "MET_CHAR") {
        type = TYPE_INT8;
        data = readRawData<char>(rawFilename, width, height, depth, nrOfComponents, isCompressed, compressedDataSize);
    } else if(typeName == "MET_UCHAR") {
        type = TYPE_UINT8;
        data = readRawData<unsigned char>(rawFilename, width, height, depth, nrOfComponents, isCompressed, compressedDataSize);
    } else if(typeName == "MET_FLOAT") {
        type = TYPE_FLOAT;
        data = readRawData<float>(rawFilename, width, height, depth, nrOfComponents, isCompressed, compressedDataSize);
    }

    if(imageIs3D) {
        output->create3DImage(width,height,depth,type,nrOfComponents,getMainDevice(),data);
    } else {
        output->create2DImage(width,height,type,nrOfComponents,getMainDevice(),data);
    }

    output->setSpacing(spacing);

    // Create transformation
    AffineTransformation T;
    T.translation() = offset;
    T.linear() = transformMatrix;
    output->getSceneGraphNode()->setTransformation(T);

    // Clean up
    deleteArray(data, type);
}
