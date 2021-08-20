#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <set>


namespace fast {

class Image;

/**
 * @brief Image compression types for ImagePyramids (TIFF)
 *
 * @ingroup wsi
 */
enum class ImageCompression {
    UNSPECIFIED,
    RAW,
    JPEG,
    JPEG2000,
    LZW, // Lossless compression
};

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
    FAST_OBJECT_V4(ImagePyramid)
    public:
        FAST_CONSTRUCTOR(ImagePyramid, int, width,, int, height,, int, channels,, int, patchWidth, = 256, int, patchHeight, = 256);
        FAST_CONSTRUCTOR(ImagePyramid, openslide_t*, fileHandle,, std::vector<ImagePyramidLevel>, levels,);
        FAST_CONSTRUCTOR(ImagePyramid, TIFF*, fileHandle,, std::vector<ImagePyramidLevel>, levels,, int, channels,);
        int getNrOfLevels();
        int getLevelWidth(int level);
        int getLevelHeight(int level);
        int getLevelTileWidth(int level);
        int getLevelTileHeight(int level);
        int getLevelTilesX(int level);
        int getLevelTilesY(int level);
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
        void setDirtyPatch(int level, int patchIdX, int patchIdY);
        void clearDirtyPatches(std::set<std::string> patches);
        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
        ~ImagePyramid();
    private:
        ImagePyramid();
        std::vector<ImagePyramidLevel> m_levels;
        ImagePyramidLevel getLevelInfo(int level);

        openslide_t* m_fileHandle = nullptr;
        TIFF* m_tiffHandle = nullptr;
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
};

}
