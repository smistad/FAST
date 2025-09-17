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
    /**
     * @brief Create a tiled image pyramid instance
     *
     * Create a tiled image pyramid using TIFF.
     *
     * @param width Full width of image pyramid
     * @param height Full height of image pyramid
     * @param channels Nr of channels of image pyramid (3 == color (RGB), 1 == grayscale)
     * @param patchWidth Width of each patch
     * @param patchHeight Height of each patch
     * @param compression Compression type to use when storing the data in the TIFF.
     * @param compressionQuality Quality of compression when using lossy compression like JPEG and JPEGXL.
     *      100 = best, 0 = worst.
     * @param dataType Data type
     * @return instance
     */
    FAST_CONSTRUCTOR(ImagePyramid,
                     int, width,,
                     int, height,,
                     int, channels,,
                     int, patchWidth, = 256,
                     int, patchHeight, = 256,
                     ImageCompression, compression, = ImageCompression::UNSPECIFIED,
                     int, compressionQuality, = 90,
                     DataType, dataType, = TYPE_UINT8
        );
#ifndef SWIG
        FAST_CONSTRUCTOR(ImagePyramid, openslide_t*, fileHandle,, std::vector<ImagePyramidLevel>, levels,);
        FAST_CONSTRUCTOR(ImagePyramid, TIFF*, fileHandle,, std::vector<ImagePyramidLevel>, levels,, int, channels,,bool, isOMETIFF, = false);
#endif
        int getNrOfLevels();
        int getLevelWidth(int level);
        int getLevelHeight(int level);
        int getLevelTileWidth(int level);
        int getLevelTileHeight(int level);
        int getLevelTilesX(int level);
        int getLevelTilesY(int level);
        float getLevelScale(int level);
        /**
         * @brief Get level for a given magnification if it exists
         * @param magnification Magnification amount (e.g. 40, 20, 10 etc.)
         * @return level
         */
        int getLevelForMagnification(float magnification);
        std::pair<int, float> getClosestLevelForMagnification(float magnification, float percentageSlack = 0.1f);
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
        ImageCompression getCompression() const;
        int getCompressionQuality() const;
        void setCompressionModels(std::shared_ptr<NeuralNetwork> compressionModel, std::shared_ptr<NeuralNetwork> decompressionModel, float outputScaleFactor = 1.0f);
        void setCompressionModel(std::shared_ptr<NeuralNetwork> compressionModel);
        void setDecompressionModel(std::shared_ptr<NeuralNetwork> decompressionModel, float outputScaleFactor = 1.0f);
        std::shared_ptr<NeuralNetwork> getCompressionModel() const;
        std::shared_ptr<NeuralNetwork> getDecompressionModel() const;
        float getDecompressionOutputScaleFactor() const;
        DataType getDataType() const;
        float getMagnification() const;
        void setMagnification(float magnification);
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
        DataType m_dataType = TYPE_UINT8;
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

        ImageCompression m_compressionFormat;
        int m_compressionQuality = -1;

        // A mutex needed to control multi-threaded reading of TIFF files
        std::mutex m_readMutex;

        std::shared_ptr<NeuralNetwork> m_compressionModel;
        std::shared_ptr<NeuralNetwork> m_decompressionModel;
        float m_decompressionOutputScaleFactor = 1.0f;

        float m_magnification = -1.0f;

        uint32_t m_JPEGTablesCount = 0;
        void* m_JPEGTablesData = nullptr;
};

}
