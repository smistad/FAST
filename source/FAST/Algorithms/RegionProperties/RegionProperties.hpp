#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
namespace fast {

class Mesh;

struct FAST_EXPORT Region {
    int pixelCount;
    float area;
    uchar label;
    Vector2f centroid;
    float perimiterLength;
    float averageRadius;
    Vector2i maxPixelPosition;
    Vector2i minPixelPosition;
    std::shared_ptr<Mesh> contourMesh;
    std::vector<Vector2i> contourPixels;
    std::vector<Vector2i> pixels;
};

FAST_SIMPLE_DATA_OBJECT(RegionList, std::vector<Region>)

/**
 * @brief Calculate properties, such as area, contour and centroid, for every segmentation region
 *
 * Inputs:
 * - 0: Image segmentation
 *
 * Outputs:
 * - 0: RegionList, a simple data object which is a vector of Region
 *
 * @ingroup segmentation
 */
class FAST_EXPORT RegionProperties : public ProcessObject {
    FAST_PROCESS_OBJECT(RegionProperties)
    public:
        FAST_CONSTRUCTOR(RegionProperties);
    protected:
        void execute() override;
};

}