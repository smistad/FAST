/**
 * @example SimpleGUI.hpp
 */
#ifndef SIMPLE_GUI_HPP_
#define SIMPLE_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>
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
		GaussianSmoothing::pointer mSmoothing;
		QLabel* mSmoothingLabel;
		QLabel* mThresholdLabel;
};

} // end namespace fast

#endif
