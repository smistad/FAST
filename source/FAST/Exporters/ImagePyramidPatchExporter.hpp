#pragma once

#include <FAST/Exporters/Exporter.hpp>

namespace fast {

class Image;

/**
 * @brief Exports an ImagePyramid to disk as a large set of image patches.
 * Each patch is stored as a PNG image with the file name indicating its position and size.
 * This exporter can handle both an ImagePyramid input and a stream of image patches.
 *
 * <h3>Inputs</h3>
 * - 0: ImagePyramid
 *
 * @ingroup exporters wsi
 * @sa ImagePyramidPatchImporter
 */
class FAST_EXPORT ImagePyramidPatchExporter : public Exporter {
    FAST_PROCESS_OBJECT(ImagePyramidPatchExporter)
    public:
        /**
         * @brief Create instance
         * @param path Path to a directory to export patches to
         * @param level Image pyramid level to extract patches from
         * @param width Width of patch
         * @param height Height of patch
         * @return instance
         */
        FAST_CONSTRUCTOR(ImagePyramidPatchExporter,
                         std::string, path,,
                         uint, level, = 0,
                         uint, width, = 512,
                         uint, height, = 512
         );
        /**
         * Path to the folder to put all tiles in. If folder does not exist, it will be created.
         * @param path
         */
        void setPath(std::string path);
        void setPatchSize(uint width, uint height);
        /**
         * Which level in the image pyramid to export patches from. Default is 0.
         * This is only used if input is an image pyramid.
         *
         * @param level
         */
        void setLevel(uint level);
        void loadAttributes() override;
    private:
        ImagePyramidPatchExporter();
        void execute() override;
        void exportPatch(std::shared_ptr<Image> patch);

        int m_level = 0;
        int m_patchWidth = 512;
        int m_patchHeight = 512;
        std::string m_path;
};

}