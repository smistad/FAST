#pragma once

#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>

namespace fast {

/**
Image sharpening by the unsharp masking method.
*/
class FAST_EXPORT ImageSharpening : public GaussianSmoothingFilter {
	FAST_OBJECT(ImageSharpening)
	public:
		void setGain(float gain);
		void loadAttributes();
	protected:
		ImageSharpening();
		void execute();

		float m_gain = 1.0;

};

}