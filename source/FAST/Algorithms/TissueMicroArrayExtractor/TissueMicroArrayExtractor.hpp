#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>

namespace fast {

class Image;
class ImagePyramid;
class RegionList;

/**
 * @brief Extract tissue micro arrays (TMAs) from a whole-slide image
 *
 * This method create a stream of images of the TMAs in a whole-slide image pyramid.
 *
 * Inputs:
 * - 0: ImagePyramid WSI
 *
 * Outputs:
 * - 0: Image stream of TMAs from input
 *
 * @ingroup wsi
 */
class FAST_EXPORT TissueMicroArrayExtractor : public Streamer {
    FAST_PROCESS_OBJECT(TissueMicroArrayExtractor)
    public:
        FAST_CONSTRUCTOR(TissueMicroArrayExtractor)
    private:
        void generateStream();
        void execute() override;

        std::shared_ptr<ImagePyramid> m_input;
        std::shared_ptr<Image> m_tissue;
        std::shared_ptr<RegionList> m_regions;
};

}