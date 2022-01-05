#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Segment the lung, airways and blood vessels from a CT using seeded region growing and morpohology
 *
 * Inputs:
 * - 0: Image a 3D CT image
 *
 * Outputs:
 * - 0: Image segmentation of lungs
 * - 1: Image segmentation of airways
 * - 2: Image segmentation of blood vessels (only if extractBloodVessels == true)
 *
 * @sa AirwaySegmentation
 * @ingroup segmentation
 */
class FAST_EXPORT LungSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(LungSegmentation)
public:
    /**
     * @brief Create instance
     * @param airwaySeedPoint Manually specify seed point for airways. By default it will automatically try to find the seed point.
     * @param lungSeedPoint Manually specify seed point for lung. By default it will automatically try to find the seed point.
     * @param extractBloodVessels Whether to extract blood vessels as well or not, Default: false
     * @return instance
     */
    FAST_CONSTRUCTOR(LungSegmentation,
                     Vector3i, airwaySeedPoint, = Vector3i::Zero(),
                     Vector3i, lungSeedPoint, = Vector3i::Zero(),
                     bool, extractBloodVessels, = false
    )
    void setAirwaySeedPoint(int x, int y, int z);
    void setAirwaySeedPoint(Vector3i seed);
    void setLungSeedPoint(int x, int y, int z);
    void setLungSeedPoint(Vector3i seed);
private:
    void execute();
    std::shared_ptr<Image> convertToHU(std::shared_ptr<Image> image);

    Vector3i findSeedVoxel(std::shared_ptr<Image> input);

    Vector3i m_airwaySeedPoint = Vector3i::Zero();
    Vector3i m_lungSeedPoint = Vector3i::Zero();
    bool m_extractBloodVessels;
};

}