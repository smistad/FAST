/**
 * @example SimpleGUI.hpp
 */
#pragma once

#include "FAST/Visualization/Window.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
#include <QLabel>

namespace fast {

class SimpleGUI : public Window {
	FAST_OBJECT_V4(SimpleGUI)
	public:
        FAST_CONSTRUCTOR(SimpleGUI)
		void updateThreshold(int value);
		void updateSmoothingParameter(int value);
	private:

		SurfaceExtraction::pointer mSurfaceExtraction;
		GaussianSmoothing::pointer mSmoothing;
		QLabel* mSmoothingLabel;
		QLabel* mThresholdLabel;
};

} // end namespace fast
