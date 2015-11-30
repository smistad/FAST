#ifndef SIMPLE_GUI_HPP_
#define SIMPLE_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
#include <QLabel>

namespace fast {

class SimpleGUI : public Window {
	FAST_OBJECT(SimpleGUI)
	public:
		void updateThreshold(int value);
		void updateSmoothingParameter(int value);
	private:
		SimpleGUI();

		SurfaceExtraction::pointer mSurfaceExtraction;
		GaussianSmoothingFilter::pointer mSmoothing;
		QLabel* mSmoothingLabel;
		QLabel* mThresholdLabel;
};

}

#endif
