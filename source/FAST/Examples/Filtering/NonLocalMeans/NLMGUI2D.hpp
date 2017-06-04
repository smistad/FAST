/**
 * Examples/Filtering/NonLocalMeans/NLMGUI2D.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#ifndef NLM_GUI2D_HPP_
#define NLM_GUI2D_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QLabel>

#include "FAST/Algorithms/NonLocalMeans/NonLocalMeans.hpp"

namespace fast {

class NLMGUI2D : public Window {
	FAST_OBJECT(NLMGUI2D)
	public:
        void updateDenoiseParameter(int value);
        void updateGroupSize(int value);
        void updateWindowSize(int value);
        void updateSigma(int value);
        void updateK(int value);
        void updateE(int value);
	private:
		NLMGUI2D();
        
		SurfaceExtraction::pointer mSurfaceExtraction;
		GaussianSmoothingFilter::pointer mSmoothing;
        NonLocalMeans::pointer nlmSmoothing;
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
        QLabel* nlmELabel;
    
};

} // end namespace fast

#endif
