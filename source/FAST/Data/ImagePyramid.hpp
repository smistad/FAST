#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <set>


namespace fast {

class Image;

/**
 * @brief Image pyramid data object
 *
 * Data object for storing large images as tiled image pyramids.
 * Storage uses virtual memory enabling the images to be larger than
 * the available RAM.
 *
 * @ingroup data wsi
 */
class FAST_EXPORT ImagePyramid : public SpatialDataObject {
    FAST_DATA_OBJECT(ImagePyramid)
    public:
        FAST_CONSTRUCTOR(ImagePyramid, int, width,, int, height,, int, channels,, int, patchWidth, = 256, int, patchHeight, = 256);
        FAST_CONSTRUCTOR(ImagePyramid, openslide_t*, fileHandle,, std::vector<ImagePyramidLevel>, levels,);
        FAST_CONSTRUCTOR(ImagePyramid, std::ifstream*, stream,, std::vector<vsi_tile_header>, tileHeaders,, std::vector<ImagePyramidLevel>, levels,, ImageCompression, compressionFormat,);
        FAST_CONSTRUCTOR(ImagePyramid, TIFF*, fileHandle,, std::vector<ImagePyramidLevel>, levels,, int, channels,,bool, isOMETIFF, = false);
        int getNrOfLevels();
        int getLevelWidth(int level);
        int getLevelHeight(int level);
        int getLevelTileWidth(int level);
        int getLevelTileHeight(int level);
        int getLevelTilesX(int level);
        int getLevelTilesY(int level);
        float getLevelScale(int level);
        /**
         * @brief Get level for a given magnification
         * @param magnification Magnification amount (e.g. 40, 20, 10 etc.)
         * @param slackPercentage Slack to allow from target magnification, given in percentage of target spacing/magnification.
         *      If distance between closest level and target magnification is larger than this, an exception is thrown.
         * @return level
         */
        int getLevelForMagnification(int magnification, float slackPercentage = 0.5f);
        int getFullWidth();
        int getFullHeight();
        int getNrOfChannels() const;
        bool isBGRA() const;
        bool usesTIFF() const;
        /**
         * Whether all patches in entire pyramid has been initialized.
         */
        bool isPyramidFullyInitialized() const;
        bool usesOpenSlide() const;
        std::string getTIFFPath() const;
        void setSpacing(Vector3f spacing);
        Vector3f getSpacing() const;
        ImagePyramidAccess::pointer getAccess(accessType type);
        std::unordered_set<std::string> getDirtyPatches();
        bool isDirtyPatch(const std::string& tileID);
        bool isOMETIFF() const;
        void setDirtyPatch(int level, int patchIdX, int patchIdY);
        void clearDirtyPatches(std::set<std::string> patches);
        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
        ~ImagePyramid();
        // Override
        DataBoundingBox getTransformedBoundingBox() const override;
        DataBoundingBox getBoundingBox() const override;
    private:
        ImagePyramid();
        std::vector<ImagePyramidLevel> m_levels;
        ImagePyramidLevel getLevelInfo(int level);

        openslide_t* m_fileHandle = nullptr;
        TIFF* m_tiffHandle = nullptr;
        bool m_tempFile = false;
        bool m_isOMETIFF = false;
        std::string m_tiffPath;

        int m_channels;
        bool m_initialized;
        /**
         * Whether all patches in entire pyramid has been initialized.
         */
        bool m_pyramidFullyInitialized;

        std::unordered_set<std::string> m_dirtyPatches;
        static int m_counter;
        std::mutex m_dirtyPatchMutex;
        Vector3f m_spacing = Vector3f::Ones();
        std::unordered_set<std::string> m_initializedPatchList; // Keep a list of initialized patches, for tiff backend

        // VSI stuff
        std::ifstream* m_vsiFileHandle;
        std::vector<vsi_tile_header> m_vsiTiles;
        ImageCompression m_compressionFormat;

        // A mutex needed to control multi-threaded reading of VSI and TIFF files
        std::mutex m_readMutex;
};

}
