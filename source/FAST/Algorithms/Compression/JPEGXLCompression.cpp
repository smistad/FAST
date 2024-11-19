#include "JPEGXLCompression.hpp"
#include <jxl/codestream_header.h>
#include <jxl/color_encoding.h>
#include <jxl/encode.h>
#include <jxl/encode_cxx.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>
#include <jxl/types.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>

namespace fast {

JPEGXLCompression::JPEGXLCompression() {

}

void* JPEGXLCompression::decompress(uchar *compressedData, std::size_t bytes, int* widthOut, int* heightOut) {
    auto runner = JxlResizableParallelRunnerMake(nullptr);

    auto decoder = JxlDecoderMake(nullptr);
    if(JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(decoder.get(),
                                            JXL_DEC_BASIC_INFO |
                                            JXL_DEC_COLOR_ENCODING |
                                            JXL_DEC_FULL_IMAGE))
        throw Exception("JPEGXL: JxlDecoderSubscribeEvents failed");

    if(JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(decoder.get(),
                                                       JxlResizableParallelRunner,
                                                       runner.get()))
        throw Exception( "JPEGXL: JxlDecoderSetParallelRunner failed");

    uint channels = 3;
    JxlBasicInfo info;
    JxlPixelFormat format = {channels, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};

    std::vector<uint8_t> ICCProfile;
    size_t width = 0;
    size_t height = 0;
    void* decompressedData;
    JxlDecoderSetInput(decoder.get(), compressedData, bytes);
    JxlDecoderCloseInput(decoder.get());

    for (;;) {
        JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());

        if(status == JXL_DEC_ERROR) {
            throw Exception( "JPEGXL: Decoder error");
        } else if(status == JXL_DEC_NEED_MORE_INPUT) {
            throw Exception( "JPEGXL: Already provided all input\n");
        } else if(status == JXL_DEC_BASIC_INFO) {
            if(JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(decoder.get(), &info))
                throw Exception( "JPEGXL: JxlDecoderGetBasicInfo failed");
            width = info.xsize;
            height = info.ysize;
            JxlResizableParallelRunnerSetThreads(
                    runner.get(),
                    JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize)
            );
        } else if(status == JXL_DEC_COLOR_ENCODING) {
            size_t ICCSize;
            if(JXL_DEC_SUCCESS != JxlDecoderGetICCProfileSize(
                        decoder.get(),
                        JXL_COLOR_PROFILE_TARGET_DATA,
                        &ICCSize))
                throw Exception("JPEGXL: JxlDecoderGetICCProfileSize failed");

            ICCProfile.resize(ICCSize);
            if(JXL_DEC_SUCCESS != JxlDecoderGetColorAsICCProfile(
                    decoder.get(), JXL_COLOR_PROFILE_TARGET_DATA,
                    ICCProfile.data(), ICCProfile.size()))
                throw Exception("JPEGXL: JxlDecoderGetColorAsICCProfile failed");
        } else if(status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            size_t bufferSize;
            if(JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(decoder.get(), &format, &bufferSize))
                throw Exception("JPEGXL: JxlDecoderImageOutBufferSize failed\n");
            if(bufferSize != width * height * channels)
                throw Exception("JPEGXL: Invalid out buffer size " + std::to_string(bufferSize) + " " +
                                std::to_string(width * height * channels));
            decompressedData = new uchar[width * height * channels];
            size_t pixelBufferSize = width * height * channels * sizeof(uchar);
            if(JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(decoder.get(), &format,
                                                              decompressedData,
                                                              pixelBufferSize)) {
                throw Exception("JPEGXL: JxlDecoderSetImageOutBuffer failed");
            }
            *widthOut = width;
            *heightOut = height;
        } else if (status == JXL_DEC_FULL_IMAGE) {
            // Nothing to do. If the image is an animation, more frames may be decoded.
        } else if (status == JXL_DEC_SUCCESS) {
            // All decoding finished
            return decompressedData;
        } else {
            throw Exception("JPEGXL: Unknown decoder status");
        }
    }
}

void JPEGXLCompression::compress(void *data, int width, int height, std::vector<uchar>* compressed) {
    auto encoder = JxlEncoderMake(nullptr);
    auto runner = JxlThreadParallelRunnerMake(
            nullptr,
            JxlThreadParallelRunnerDefaultNumWorkerThreads()
    );
    if(JXL_ENC_SUCCESS != JxlEncoderSetParallelRunner(encoder.get(),
                                                       JxlThreadParallelRunner,
                                                       runner.get()))
        throw Exception("JPEGXL: JxlEncoderSetParallelRunner failed");

    JxlPixelFormat pixelFormat = {3, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};

    JxlBasicInfo basicInfo;
    JxlEncoderInitBasicInfo(&basicInfo);
    basicInfo.xsize = width;
    basicInfo.ysize = height;
    int channels = 3;
    basicInfo.bits_per_sample = 32;
    basicInfo.exponent_bits_per_sample = 8;
    basicInfo.uses_original_profile = JXL_FALSE;
    if(JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(encoder.get(), &basicInfo))
        throw Exception("JPEGXL: JxlEncoderSetBasicInfo failed");

    JxlColorEncoding colorEncoding = {};
    JXL_BOOL isGrayscale = TO_JXL_BOOL(pixelFormat.num_channels < 3);
    JxlColorEncodingSetToSRGB(&colorEncoding, isGrayscale);
    if(JXL_ENC_SUCCESS != JxlEncoderSetColorEncoding(encoder.get(), &colorEncoding))
        throw Exception("JPEGXL: JxlEncoderSetColorEncoding failed");

    auto frameSettings = JxlEncoderFrameSettingsCreate(encoder.get(), nullptr);

    if(JXL_ENC_SUCCESS != JxlEncoderAddImageFrame(frameSettings, &pixelFormat,
                            static_cast<const void*>(data),
                            sizeof(uchar) * width*height*channels))
        throw Exception("JPEGXL: JxlEncoderAddImageFrame failed");
    JxlEncoderCloseInput(encoder.get());

    compressed->resize(64);
    uint8_t* next_out = compressed->data();
    size_t avail_out = compressed->size() - (next_out - compressed->data());
    JxlEncoderStatus processResult = JXL_ENC_NEED_MORE_OUTPUT;
    while(processResult == JXL_ENC_NEED_MORE_OUTPUT) {
        processResult = JxlEncoderProcessOutput(encoder.get(), &next_out, &avail_out);
        if(processResult == JXL_ENC_NEED_MORE_OUTPUT) {
            size_t offset = next_out - compressed->data();
            compressed->resize(compressed->size() * 2);
            next_out = compressed->data() + offset;
            avail_out = compressed->size() - offset;
        }
    }
    compressed->resize(next_out - compressed->data());
    if(JXL_ENC_SUCCESS != processResult)
        throw Exception( "JPEGXL: JxlEncoderProcessOutput failed");
}

}