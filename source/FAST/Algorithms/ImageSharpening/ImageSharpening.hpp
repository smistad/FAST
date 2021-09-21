#pragma once

#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>

namespace fast {

/**
* @brief Image sharpening by the unsharp masking method.
 *
 * @ingroup filter
*/
class FAST_EXPORT ImageSharpening : public GaussianSmoothing {
	FAST_PROCESS_OBJECT(ImageSharpening)
	public:
        /**
         * @brief Create instance
         * @param gain
         * @param stdDev Standard deviation of convolution kernel
         * @param maskSize Size of convolution filter/mask. Must be odd.
         *      If 0 filter size is determined automatically from standard deviation
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageSharpening,
                         float, gain, = 1.0f,
                         float, stddev, = 0.5f,
                         uchar, maskSize, = 0
        );
		void setGain(float gain);
		void loadAttributes();
	protected:
		void execute();

		float m_gain = 1.0;

};

}