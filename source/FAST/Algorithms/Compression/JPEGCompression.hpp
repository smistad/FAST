#pragma once

#include <FAST/Data/DataTypes.hpp>

namespace fast {

/**
 * @brief Class for JPEG image compression
 *
 * Only supports 8 bit RGB images for now.
 */
class JPEGCompression {
public:
    explicit JPEGCompression(uint32_t JPEGTableCount = 0, void* tableData = nullptr);
    void compress(void* data, int width, int height, std::vector<uint8_t>* compressedData, int quality = 90);
    /**
     * @brief Decompress
     * @param compressedData
     * @param bytes
     * @param widthOut
     * @param heightOut
     * @param outputBuffer if nullptr this buffer will be used to store data
     * @return decompressed data
     */
    void* decompress(uchar* compressedData, std::size_t bytes, int* widthOut, int* heightOut, uchar* outputBuffer = nullptr);

private:
    uint32_t m_tableCount = 0;
    const void* m_tableData = nullptr;
};
}