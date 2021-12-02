#pragma once

#include <FAST/Streamers/FileStreamer.hpp>

namespace fast {

class Image;

/**
 * @brief Stream a sequence of image files from disk
 *
 * Images has to be stored with an index before the file extension.
 * E.g. some_name_0.mhd, some_name_1.mhd, some_name_2.mhd ...
 * or some_name_0.png, some_name_1.png, some_name_2.png ...
 *
 * This streamer uses the ImageFileImporter to read the images from disk.
 *
 * <h3>Output ports</h3>
 * - 0: Image
 *
 * @ingroup streamers
 */
class FAST_EXPORT ImageFileStreamer : public FileStreamer {
    FAST_PROCESS_OBJECT(ImageFileStreamer)
    public:
        /**
         * @brief Create a ImageFileStreamer instance
         *
         * @param filenameFormat String of path and format of images. E.g. /path/to/files/frame_#.mhd. The hash sign #
         * will be replaced by an index
         * @param loop Whether to loop the recording or not
         * @param useTimestamps Whether to use timestamps in image files (if available) when streaming, or just stream as fast as possible
         * @param framerate If framerate is > 0, this framerate will be used for streaming the images
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageFileStreamer,
                         std::string, filenameFormat,,
                         bool, loop, = false,
                         bool, useTimestamps, = true,
                         int, framerate, = -1
        );
    protected:
        DataObject::pointer getDataFrame(std::string filename) override;

        ImageFileStreamer();

};

} // end namespace fast
