#include <fstream>
#include "NIFTIImporter.hpp"
#include <FAST/Data/Image.hpp>
#include <zlib/zlib.h>

namespace fast {

NIFTIImporter::NIFTIImporter() {
    createOutputPort(0);
}

NIFTIImporter::NIFTIImporter(std::string filename) : FileImporter(filename)  {
    createOutputPort(0);
}

class NIFTIFile {
    public:
        virtual void seek(uint64_t position) = 0;
        virtual void read(char* data, uint64_t bytes) = 0;
};

class Uncompressed : public NIFTIFile {
    public:
        Uncompressed(std::string filename) {
            m_stream.open(filename, std::ios_base::binary);
            if(!m_stream.is_open())
                throw Exception("Unable to open file " + filename);
        }
        void seek(uint64_t position) {
            m_stream.seekg(position);
        }
        void read(char* data, uint64_t bytes) {
            m_stream.read(data, bytes) ;
        }
    private:
        std::ifstream m_stream;
};

class Compressed : public NIFTIFile {
    public:
        Compressed(std::string filename) {
            m_file = gzopen(filename.c_str(), "r");
        }
        void seek(uint64_t position) {
            gzseek(m_file, position, SEEK_SET);
        }
        void read(char* data, uint64_t bytes) {
            gzread(m_file, data, bytes);
        }
        ~Compressed() {
            gzclose(m_file);
        }
    private:
        gzFile m_file;
    };

template <class T>
inline void read(std::unique_ptr<NIFTIFile>& file, int64_t offset, T& data) {
    file->seek(offset);
    file->read((char*)&data, sizeof(T));
}

template <class T>
inline void read(std::unique_ptr<NIFTIFile>& file, T& data) {
    file->read((char*)&data, sizeof(T));
}

void NIFTIImporter::execute() {
    if(m_filename.empty())
        throw Exception("You must give a filename to NIFTImporter");
    if(!fileExists(m_filename))
        throw FileNotFoundException(m_filename);

    std::unique_ptr<NIFTIFile> file;
    if(m_filename.substr(m_filename.size()-3) == ".gz") {
        //gzopen(m_filename.c_str(), "r");
        // Use gzread, gzseek etc.
        file = std::make_unique<Compressed>(m_filename);
    } else {
        file = std::make_unique<Uncompressed>(m_filename);
    }

    int headerSize;
    read(file, headerSize);

    // See table at https://brainder.org/2012/09/23/the-nifti-file-format/
    std::map<int16_t, DataType> imageTypeMap = {
            {4, TYPE_INT16},
            {8, TYPE_UINT8},
            {16, TYPE_FLOAT},
            {256, TYPE_INT8},
            {512, TYPE_UINT16},
    };

    if(headerSize == 348) {
        int16_t dataType;
        read(file, 70, dataType);

        if(imageTypeMap.count(dataType) == 0) {
            throw Exception("Unsupported data type in NIFTI file: " + std::to_string(dataType));
        }
        auto imageType = imageTypeMap[dataType];

        // Read dimensions
        int16_t dims[8];
        read(file, 40, dims);
        //for(int i = 0; i < 8; ++i)
        //    std::cout << "dims: " << dims[i] << std::endl;
        // dims[0] is the number of dimensions

        // Get spacing info
        float pixdim[8];
        read(file, 76, pixdim);
        char unit;
        read(file, 123, unit);
        std::map<char, float> scalingMap = {{1, 1000.0f}, {2, 1.0f}, {3, 0.001}};
        if(scalingMap.count(unit) == 0)
            throw Exception("Unsupported unit in NIFT file " + std::to_string(unit));
        float scaling = scalingMap[unit];
        Vector3f spacing(pixdim[1]*scaling, pixdim[2]*scaling, pixdim[3]*scaling);

        // Read pixel data
        float dataOffset;
        read(file, 108, dataOffset);
        int64_t dataSize = dims[1]*dims[2]*dims[3]*getSizeOfDataType(imageType, 1);
        auto data = make_uninitialized_unique<char>(dataSize);
        file->seek(dataOffset);
        file->read((char*)data.get(), dataSize);
        auto image = Image::create(dims[1], dims[2], dims[3], imageType, 1, std::move(data));
        image->setSpacing(spacing);
        addOutputData(image);
    } else if(headerSize == 540) {
        reportInfo() << "File was in NIFTI 2 format" << reportEnd();
        int16_t dataType;
        read(file, 12, dataType);

        if(imageTypeMap.count(dataType) == 0) {
            throw Exception("Unsupported data type in NIFTI file: " + std::to_string(dataType));
        }
        auto imageType = imageTypeMap[dataType];

        int64_t dims[8];
        read(file, 16, dims);
        //for(int i = 0; i < 8; ++i)
        //    std::cout << "dims: " << dims[i] << std::endl;

        // Get spacing info
        double pixdim[8];
        read(file, 104, pixdim);
        int32_t unit;
        read(file, 500, unit);
        std::map<int32_t, float> scalingMap = {{1, 1000.0f}, {2, 1.0f}, {3, 0.001}};
        if(scalingMap.count(unit) == 0)
            throw Exception("Unsupported unit in NIFT file " + std::to_string(unit));
        float scaling = scalingMap[unit];
        Vector3f spacing(pixdim[1]*scaling, pixdim[2]*scaling, pixdim[3]*scaling);

        // Read pixel data
        int64_t dataOffset;
        read(file, 168, dataOffset);
        int64_t dataSize = dims[1]*dims[2]*dims[3]*getSizeOfDataType(imageType, 1);
        auto data = make_uninitialized_unique<char>(dataSize);
        file->seek(dataOffset);
        file->read((char*)data.get(), dataSize);
        auto image = Image::create(dims[1], dims[2], dims[3], imageType, 1, std::move(data));
        image->setSpacing(spacing);
        addOutputData(image);
    } else {
        throw Exception("Unexpected header size in NIFTI file: " + std::to_string(headerSize));
    }
}

}