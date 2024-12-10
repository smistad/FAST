#pragma once

#include <FAST/Data/DataTypes.hpp>

namespace fast {

/**
 * @brief Class for JPEG XL image compression
 *
 * Only supports 8 bit RGB images for now.
 */
class JPEGXLCompression {
    public:
        JPEGXLCompression();
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

};
}
