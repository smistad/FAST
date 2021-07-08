#pragma once

#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>

namespace fast {

/**
Image sharpening by the unsharp masking method.
*/
class FAST_EXPORT ImageSharpening : public GaussianSmoothing {
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