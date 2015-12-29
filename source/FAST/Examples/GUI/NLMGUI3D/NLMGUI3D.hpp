/**
 * Examples/GUI/SimpleGUI/SimpleGUI.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#ifndef NLM_GUI_HPP_
#define NLM_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
#include <FAST/Algorithms/NoneLocalMeans/NoneLocalMeans.hpp>
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QLabel>

namespace fast {

class NLMGUI3D : public Window {
	FAST_OBJECT(NLMGUI3D)
	public:
		void updateThreshold(int value);
        void updateDenoiseParameter(int value);
        void updateGroupSize(int value);
        void updateWindowSize(int value);
        void updateSigma(int value);
        void updateK(int value);
	private:
		NLMGUI3D();
        
		SurfaceExtraction::pointer mSurfaceExtraction;
		GaussianSmoothingFilter::pointer mSmoothing;
        NoneLocalMeans::pointer nlmSmoothing;
        ImageRenderer::pointer renderer;
        ImageRenderer::pointer rendererOrig;
        View* view;
        View* viewOrig;
		//QLabel* mSmoothingLabel;
		QLabel* mThresholdLabel;
        QLabel* nlmStrengthLabel;
        QLabel* nlmGroupSizeLabel;
        QLabel* nlmWindowSizeLabel;
        QLabel* nlmSigmaLabel;
        QLabel* nlmKLabel;
        QLabel* timerLabel;
        
    
};

} // end namespace fast

#endif
