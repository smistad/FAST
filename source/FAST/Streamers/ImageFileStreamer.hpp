#pragma once

#include <FAST/Streamers/FileStreamer.hpp>

namespace fast {

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
    FAST_OBJECT(ImageFileStreamer)
    protected:
        DataObject::pointer getDataFrame(std::string filename) override;

        ImageFileStreamer();

};

} // end namespace fast
