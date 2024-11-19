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
        void compress(void* data, int width, int height, std::vector<uint8_t>* compressedData);
        static void* decompress(uchar* compressedData, std::size_t bytes, int* widthOut, int* heightOut);
};
}
