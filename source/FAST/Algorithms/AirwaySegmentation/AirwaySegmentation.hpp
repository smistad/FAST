#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;

/**
 * @brief Segment airways from CT using seeded region growing with explosion control
 *
 * An example of this can be found in @ref airwaySegmentation.cpp
 *
 * Inputs:
 * - 0: Image - CT thorax 3D image
 *
 * Outputs:
 * - 0: Image - Segmentation image
 *
 * @sa CenterlineExtraction
 */
class FAST_EXPORT AirwaySegmentation : public ProcessObject {
	FAST_PROCESS_OBJECT(AirwaySegmentation)
	public:
        /**
         * @brief Create AirwaySegmentation instance
         *
         * @param smoothing Standard deviation of Gaussian smoothing to apply before segmentation.
         * A higher value can be used for low dose CT.
         * @param seed Manually set seed point. If set to (-1,-1,-1), it will try to find seed point automatically.
         * @return instance
         */
        FAST_CONSTRUCTOR(AirwaySegmentation,
                         float, smoothing, = 0.5f,
                         Vector3i, seed, = Vector3i(-1,-1,-1)
        );
        /**
         * @brief Set manual seed point for region growing
         * @param x
         * @param y
         * @param z
         */
	    void setSeedPoint(int x, int y, int z);
	    /**
	     * @brief Set manual seed point for region growing
	     * @param seed
	     */
		void setSeedPoint(Vector3i seed);
		/**
		 * @brief Standard deviation of Gaussian smoothing for preprocessing
		 *
		 * Set the standard deviation value of the GaussianSmoothing performed before segmentation.
		 * Default is 0.5. A higher value can be used for low dose CT.
		 * @param sigma standard deviation
		 */
		void setSmoothing(float sigma);
		void loadAttributes() override;
	private:
		void execute();
		static Vector3i findSeedVoxel(std::shared_ptr<Image> volume);
		std::shared_ptr<Image> convertToHU(std::shared_ptr<Image> image);
		void morphologicalClosing(std::shared_ptr<Image> segmentation);

		Vector3i mSeedPoint;
		float mSmoothingSigma = 0.5;
        bool mUseManualSeedPoint = false;
};

}