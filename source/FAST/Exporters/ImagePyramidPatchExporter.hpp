#pragma once

namespace fast {

#include <FAST/ProcessObject.hpp>

/**
 * Exports an image pyramid to disk as a large set of image tile.
 * Each tile is stored as a PNG image with the file name indicating its position and size.
 * This exporter can handle both an ImagePyramid input and a stream of image patches.
 */
class ImagePyramidTileExporter : public ProcessObject {
    FAST_OBJECT(ImagePyramidTileExporter)
    public:
        /**
         * Path to the folder to put all tiles in. If folder does not exist, it will be created.
         * @param path
         */
        void setPath(std::string path);
        void setPatchSize();
        void setLevel(uint level);
    private:
        void execute() override;
};

}