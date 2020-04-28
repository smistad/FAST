#pragma once

#include <FAST/Data/SpatialDataObject.hpp>
#include <FAST/Data/Access/Access.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <set>

// Forward declare

namespace fast {

class Image;

/**
 * Data object for storing large images as tiled image pyramids.
 * Storage uses virtual memory enabling the images to be larger than
 * the available RAM.
 */
class FAST_EXPORT ImagePyramid : public SpatialDataObject {
    FAST_OBJECT(ImagePyramid)
    public:
        void create(int width, int height, int channels, int levels = -1);
        void create(openslide_t* fileHandle, std::vector<Level> levels);
        int getNrOfLevels();
        int getLevelWidth(int level);
        int getLevelHeight(int level);
        int getLevelPatches(int level);
        int getFullWidth();
        int getFullHeight();
        int getNrOfChannels() const;
        ImagePyramidAccess::pointer getAccess(accessType type);
        std::set<std::string> getDirtyPatches();
        void setDirtyPatch(int level, int patchIdX, int patchIdY);
        void clearDirtyPatches(std::set<std::string> patches);
        void free(ExecutionDevice::pointer device) override;
        void freeAll() override;
        ~ImagePyramid();
    private:
        ImagePyramid();
        std::vector<Level> m_levels;

        openslide_t* m_fileHandle = nullptr;

        int m_channels;
        bool m_initialized;

        std::set<std::string> m_dirtyPatches;
        static int m_counter;
        std::mutex m_dirtyPatchMutex;
};

}
