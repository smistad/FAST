#include "MetaImageImporter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"
#include <fstream>
#include <set>
#include <zlib/zlib.h>
using namespace fast;

MetaImageImporter::MetaImageImporter() {
    m_filename = "";
    mIsModified = true;
    createOutputPort(0, "Image");
    setMainDevice(Host::getInstance()); // Default is to put image on host
}

MetaImageImporter::MetaImageImporter(std::string filename) : FileImporter(filename) {
    createOutputPort(0, "Image");
    setMainDevice(Host::getInstance()); // Default is to put image on host
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
static std::unique_ptr<T[]> readRawData(std::string rawFilename, std::size_t voxels, unsigned int nrOfComponents, bool compressed, std::size_t compressedFileSize) {
    auto data = make_uninitialized_unique<T[]>(voxels*nrOfComponents);
    if(compressed) {
        // Read compressed data
        std::ifstream file(rawFilename, std::ifstream::binary | std::ifstream::in);
        if(!file.is_open())
            throw FileNotFoundException(rawFilename);

        // Determine the file length
        file.seekg(0, std::ios_base::end);
        std::size_t size = file.tellg();
        file.seekg(0, std::ios_base::beg);

        auto fileData = make_uninitialized_unique<Bytef[]>(size);
        file.read((char*)&fileData[0], size);
        file.close();

        unsigned long uncompressedSize = sizeof(T)*voxels*nrOfComponents;
		int z_result = uncompress(
            (Bytef*)data.get(),       // destination for the uncompressed
                                    // data.  This should be the size of
                                    // the original data, which you should
                                    // already know.

            (uLongf *)&uncompressedSize,  // length of destination (uncompressed)
                                    // buffer

            fileData.get(),   // source buffer - the compressed data

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
    } else {
        std::ifstream file(rawFilename, std::ifstream::binary | std::ifstream::in);
        if(!file.is_open())
            throw FileNotFoundException(rawFilename);

        // Determine the file length
        file.seekg(0, std::ios_base::end);
        std::size_t size = file.tellg();
        file.seekg(0, std::ios_base::beg);
        std::size_t expectedSize = voxels*nrOfComponents*sizeof(T);
        if(size != expectedSize)
            throw Exception("Unexpected file system when opening" + rawFilename + " expected: " + std::to_string(expectedSize) + " got: " + std::to_string(size));

        file.read((char*)data.get(), size);
        file.close();
    }
    return data;
}

void MetaImageImporter::execute() {
    if(m_filename == "")
        throw Exception("Filename was not set in MetaImageImporter");

    // Open and parse mhd file
    std::fstream mhdFile;
    mhdFile.open(m_filename.c_str(), std::fstream::in);
    if(!mhdFile.is_open())
        throw FileNotFoundException(m_filename);
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

    VectorXui size;
    unsigned int nrOfComponents = 1;
    uint64_t timestamp = 0;

    Vector3f spacing(1,1,1), offset(0,0,0), centerOfRotation(0,0,0);
    Matrix3f transformMatrix = Matrix3f::Identity();
    bool isCompressed = false;
    std::size_t compressedDataSize = 0;
    std::map<std::string, std::string> metadata;

    // Blacklist of keys to avoid importing as metadata
    std::set<std::string> blacklist = {
        "NDims",
        "ObjectType",
        "BinaryData"
    };

    do{
        std::getline(mhdFile, line);
        trim(line);
        if(line.size() == 0) // line is empty
            continue;
        int firstSpace = line.find(" ");
        std::string key = line.substr(0, firstSpace);
        trim(key);
        int equalSignPos = line.find("=");
        std::string value = line.substr(equalSignPos+1);
        trim(value);
        if(key == "DimSize") {
            std::vector<std::string> values = split(value);

            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());

            if(imageIs3D) {
                if(values.size() != 3)
                    throw Exception("DimSize in MetaImage file did not contain 3 numbers");
                int width = std::stoi(values[0]);
                int height = std::stoi(values[1]);
                int depth = std::stoi(values[2]);
                size = Vector3ui(width, height, depth);
            } else {
                int width = std::stoi(values[0]);
                int height = std::stoi(values[1]);
                if(values.size() != 2)
                    throw Exception("DimSize in MetaImage file did not contain 2 numbers");
                size = Vector2ui(width, height);
            }
            sizeFound = true;
        } else if(key == "CompressedData" && value == "True") {
            isCompressed = true;
        } else if(key == "CompressedDataSize") {
            compressedDataSize = std::stoi(value);
        } else if(key == "ElementDataFile") {
            rawFilename = value;
            rawFilenameFound = true;

            // Remove any trailing spaces
            int pos = rawFilename.find(" ");
            if(pos > 0)
            rawFilename = rawFilename.substr(0,pos);

            // Get path name
            pos = m_filename.rfind('/');
#ifdef WIN32
			// Windows often use \ instead of / in pathnames
			int pos2 = m_filename.rfind('\\');
			if(pos2 > pos) {
				pos = pos2;
			}
#endif
            if(pos > 0)
                rawFilename = m_filename.substr(0,pos+1) + rawFilename;
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
            nrOfComponents = std::stoi(value.c_str());
            if(nrOfComponents <= 0)
                throw Exception("Error in reading the number of components in the MetaImageImporter");
        } else if(key == "ElementSpacing") {
            std::vector<std::string> values = split(value);
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            try {
                if(imageIs3D) {
                    if(values.size() != 3)
                        throw Exception("ElementSpacing in MetaImage file did not contain 3 numbers");
                    spacing[0] = std::stof(values[0]);
                    spacing[1] = std::stof(values[1]);
                    spacing[2] = std::stof(values[2]);
                } else {
                    if(values.size() != 2 && values.size() != 3)
                        throw Exception("ElementSpacing in MetaImage file did not contain 2 or 3 numbers");

                    spacing[0] = std::stof(values[0]);
                    spacing[1] = std::stof(values[1]);
                    if(values.size() == 2) {
                        spacing[2] = 1;
                    } else {
                        spacing[2] = std::stof(values[2]);
                    }
                }
            } catch(std::out_of_range &e) {
                reportWarning() << "Out of range exception occured when center of rotation values from metaimage file" << reportEnd();
            }
        } else if(key == "CenterOfRotation") {
            //reportInfo() << "WARNING: CenterOfRotation in Metaimage file ignored" << Reporter::end();
            std::vector<std::string> values = split(value);
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            try {
                if(imageIs3D) {
                    if(values.size() != 3)
                        throw Exception("CenterOfRotation in MetaImage file did not contain 3 numbers");
                    centerOfRotation[0] = std::stof(values[0]);
                    centerOfRotation[1] = std::stof(values[1]);
                    centerOfRotation[2] = std::stof(values[2]);
                } else {
                    if(values.size() != 2 && values.size() != 3)
                        throw Exception("CenterOfRotation in MetaImage file did not contain 2 or 3 numbers");

                    centerOfRotation[0] = std::stof(values[0]);
                    centerOfRotation[1] = std::stof(values[1]);
                    if(values.size() == 2) {
                        centerOfRotation[2] = 0;
                    } else {
                        centerOfRotation[2] = std::stof(values[2]);
                    }
                }
            } catch(std::out_of_range &e) {
                reportWarning() << "Out of range exception occured when center of rotation values from metaimage file" << reportEnd();
            }
        } else if(key == "Offset" || key == "Origin" || key == "Position") {
            std::vector<std::string> values = split(value);
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());

            try {
                if(imageIs3D){
                    if(values.size() != 3) {
                        reportError() << "Offset/Origin/Position in MetaImage file did not contain 3 numbers" << reportEnd();
                        reportError() << "Ignoring" << reportEnd();
                    }
                    offset[0] = std::stof(values[0].c_str());
                    offset[1] = std::stof(values[1].c_str());
                    offset[2] = std::stof(values[2].c_str());
                } else {
                    if(values.size() != 2 && values.size() != 3)
                        throw Exception("Offset/Origin/Position in MetaImage file did not contain 2 or 3 numbers");

                    offset[0] = std::stof(values[0].c_str());
                    offset[1] = std::stof(values[1].c_str());
                    if(values.size() == 2) {
                        offset[2] = 0;
                    } else {
                        offset[2] = std::stof(values[2].c_str());
                    }
                }
            } catch(std::out_of_range &e) {
                reportWarning() << "Out of range exception occured when reading offset values from metaimage file" << reportEnd();
            }
        } else if(key == "TransformMatrix" || key == "Rotation" || key == "Orientation") {
            std::vector<std::string> values = split(value);
            // Remove any empty values:
            values.erase(std::remove(values.begin(), values.end(), ""), values.end());
            if (values.size() != 9) {
                reportError()
                        << "Encountered a transform/orientation/rotation matrix with incorrect number of elements in the MetaImageImporter"
                        << reportEnd();
                reportError() << "Ignoring" << reportEnd();
            } else {

                try {
                    for (unsigned int i = 0; i < 3; i++) {
                        for (unsigned int j = 0; j < 3; j++) {
                            transformMatrix(j, i) = std::stof(values[j + i * 3].c_str());
                        }
                    }
                } catch (std::out_of_range &e) {
                    reportWarning()
                            << "Value out of range exception occured when reading transform matrix values from metaimage file"
                            << reportEnd();
                }
            }
        } else if(key == "Timestamp") {
            timestamp = std::stoull(value);
        } else {
            // Unknown metadata, collect and add to image object
            if(blacklist.count(key) == 0)
                metadata[key] = value;
        }

    } while(!mhdFile.eof());

    mhdFile.close();
    if(!sizeFound || !rawFilenameFound || !typeFound || !dimensionsFound)
        throw Exception("Error reading the mhd file", __LINE__, __FILE__);


    Image::pointer output;
    std::size_t voxels = size.x()*size.y();
    if(size.size() == 3)
        voxels *= size.z();
    if(typeName == "MET_SHORT" || typeName == "MET_INT") {
        std::unique_ptr<short[]> data;
        if(typeName == "MET_SHORT") {
            data = std::move(readRawData<short>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize));
        } else {
            reportWarning() << "Converting original dataset of type MET_INT (32 bit) to short (16 bit) overflow may occur." << reportEnd();
            auto tmp = readRawData<int>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize);
            auto tmp2 = make_uninitialized_unique<short[]>(voxels*nrOfComponents);
            for(int i = 0; i < voxels*nrOfComponents; ++i)
                tmp2[i] = (short)tmp[i];

            data = std::move(tmp2);
        }
        output = Image::create(size,TYPE_INT16,nrOfComponents,getMainDevice(),std::move(data));

    } else if(typeName == "MET_USHORT" || typeName == "MET_UINT") {
        std::unique_ptr<ushort[]> data;
        if(typeName == "MET_USHORT") {
            data = std::move(readRawData<unsigned short>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize));
        } else {
            reportWarning() << "Converting original dataset of type MET_UINT (32 bit) to unsigned short (16 bit) overflow may occur." << reportEnd();
            auto tmp = readRawData<unsigned int>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize);
            auto tmp2 = make_uninitialized_unique<ushort[]>(voxels*nrOfComponents);
            for(int i = 0; i < voxels*nrOfComponents; ++i)
                tmp2[i] = (unsigned short)tmp[i];

            data = std::move(tmp2);
        }
        output = Image::create(size,TYPE_UINT16,nrOfComponents,getMainDevice(),std::move(data));
    } else if(typeName == "MET_CHAR") {
        auto data = readRawData<char>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize);
        output = Image::create(size,TYPE_INT8,nrOfComponents,getMainDevice(),std::move(data));
    } else if(typeName == "MET_UCHAR") {
        auto data = readRawData<unsigned char>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize);
        output = Image::create(size,TYPE_UINT8,nrOfComponents,getMainDevice(),std::move(data));
    } else if(typeName == "MET_FLOAT") {
        auto data = readRawData<float>(rawFilename, voxels, nrOfComponents, isCompressed, compressedDataSize);
        output = Image::create(size,TYPE_FLOAT,nrOfComponents,getMainDevice(),std::move(data));
    }

    output->setSpacing(spacing);

    // Add metadata
    output->setMetadata(metadata);

    output->setCreationTimestamp(timestamp);

    // Create transformation
	Affine3f matrix = Affine3f::Identity();
	matrix.translation() = offset;
	matrix.linear() = transformMatrix;
    output->getSceneGraphNode()->setTransform(matrix);
    addOutputData(0, output);
}
