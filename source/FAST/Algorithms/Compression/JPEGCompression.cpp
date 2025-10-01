#include "JPEGCompression.hpp"
#include <jpeglib.h>

namespace fast {


inline void jpegErrorExit(j_common_ptr cinfo) {
    char jpegLastErrorMsg[JMSG_LENGTH_MAX];
    // Create message
    ( *( cinfo->err->format_message ) ) ( cinfo, jpegLastErrorMsg );
    throw std::runtime_error( jpegLastErrorMsg );
}

void* JPEGCompression::decompress(uchar* compressedData, std::size_t bytes, int* widthOut, int* heightOut, uchar* outputBuffer) {
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr; //error handling
    //jerr.error_exit = jpegErrorExit;
    cinfo.err = jpeg_std_error(&jerr);
    try {
        jpeg_create_decompress(&cinfo);
        if(m_tableCount > 0) {
            jpeg_mem_src(&cinfo, (const uchar*)m_tableData, m_tableCount);
            if(jpeg_read_header(&cinfo, FALSE) != JPEG_HEADER_TABLES_ONLY) {
                throw Exception("Error setting JPEG tables");
            }
        }
        jpeg_mem_src(&cinfo, compressedData, bytes);
        int ret = jpeg_read_header(&cinfo, TRUE);
        if(ret != JPEG_HEADER_OK) {
            throw Exception("Unable to read JPEG header.");
        }
        jpeg_start_decompress(&cinfo); // output_width and output_height is available after this call

        *widthOut = cinfo.output_width;
        *heightOut = cinfo.output_height;
        //cinfo.jpeg_color_space = JCS_YCbCr;
        //cinfo.jpeg_color_space = JCS_RGB;
        if(outputBuffer == nullptr)
            outputBuffer = new uchar[(*widthOut) * (*heightOut) * 3];
        unsigned char* line = (uchar*)outputBuffer;
        while(cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines (&cinfo, &line, 1);
            line += cinfo.output_components*cinfo.output_width;
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
    } catch(std::exception &e) {
        jpeg_destroy_decompress( &cinfo );
        throw Exception("JPEG error: " + std::string(e.what())); // or return an error code
    }
    return outputBuffer;
}

void JPEGCompression::compress(void *data, int width, int height, std::vector<uint8_t> *compressedData, int quality) {
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Set up memory
#ifdef WIN32
    unsigned long resultSize = 0;
#else
    std::size_t resultSize = 0;
#endif
    uchar* resultBuffer = nullptr;
    jpeg_mem_dest(&cinfo, &resultBuffer, &resultSize);

    // Set parameters
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    // Use 4:4:4 subsampling, default is 4:2:0
    //cinfo.comp_info[0].h_samp_factor = cinfo.comp_info[0].v_samp_factor = 1;

    jpeg_start_compress(&cinfo, TRUE);

    auto line = (uchar*)data;
    while(cinfo.next_scanline < cinfo.image_height) {
        jpeg_write_scanlines(&cinfo, &line, 1);
        line += cinfo.image_width*cinfo.input_components;
    }
    jpeg_finish_compress(&cinfo);

    // Copy buffer after jpeg_finish_compress
    // TODO can we avoid this copy?
    compressedData->resize(resultSize);
    std::copy(resultBuffer, resultBuffer + resultSize, compressedData->begin());
    free(resultBuffer);
    jpeg_destroy_compress(&cinfo);
}
JPEGCompression::JPEGCompression(uint32_t tableCount, void* tableData) {
    m_tableCount = tableCount;
    m_tableData = tableData;
}

} // End namespace