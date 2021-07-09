#pragma once

#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>

namespace fast {

/**
* @brief Image sharpening by the unsharp masking method.
*/
class FAST_EXPORT ImageSharpening : public GaussianSmoothing {
	FAST_PROCESS_OBJECT(ImageSharpening)
	public:
        FAST_CONSTRUCTOR(ImageSharpening,
                         float, gain, = 1.0f,
                         float, stddev, = 0.5f,
                         uchar, maskSize, = 0
        )
		void setGain(float gain);
		void loadAttributes();
	protected:
		void execute();

		float m_gain = 1.0;

};

}