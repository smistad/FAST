#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class Image;

/**
 * @brief Segment airways from CT using seeded region growing with explosion control
 *
 * Inputs:
 * - 0: Image CT thorax 3D image
 *
 * Outputs:
 * - 0: Segmentation
 */
class FAST_EXPORT AirwaySegmentation : public ProcessObject {
	FAST_PROCESS_OBJECT(AirwaySegmentation)
	public:
        FAST_CONSTRUCTOR(AirwaySegmentation, float, smoothing, = 0.5f, Vector3i, seed, = Vector3i(-1,-1,-1))
        /**
         * Set seed point for region growing
         * @param x
         * @param y
         * @param z
         */
	    void setSeedPoint(int x, int y, int z);
	    /**
	     * Set seed point for region growing
	     * @param seed
	     */
		void setSeedPoint(Vector3i seed);
		/**
		 * Set the sigma value of the gaussian smoothing performed before segmentation.
		 * Default is 0.5. A higher value can be used for low dose CT.
		 * @param sigma
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