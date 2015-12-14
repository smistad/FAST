/**
 * Examples/GUI/SimpleGUI/SimpleGUI.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#ifndef SIMPLE_FILTERING_GUI_HPP_
#define SIMPLE_FILTERING_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
//#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
//#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"
#include <QLabel>

#include <FAST/Importers/ImageFileImporter.hpp>
//#include "FAST/Importers/I
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"

namespace fast {

class SimpleFilteringGUI : public Window {
    FAST_OBJECT(SimpleFilteringGUI)
	public:
		void updateFilterSize(int value);
		void updateFilterType(int value);
        void updateGaussStd(float value);
        void updateRunType(int value);
        void updateInputImage(int value);

	private:
        SimpleFilteringGUI();

        std::string numToRunType(int num);
        std::string getInputFilename(int inputnum);
        void updateRuntimes(Filtering::pointer filter);
        void saveImage();//ProcessObjectPort fromPort);

        Filtering::pointer mBoxFilter;
        GaussianFiltering::pointer mGaussian;
        SobelFiltering::pointer mSobelX;
        SobelFiltering::pointer mSobelY;
        AverageImages::pointer mSobelTot; //adds the two sobel images together name to change


        QLabel* mInputImageLabel;
		QLabel* mFilterSizeLabel; 
        QLabel* mFilterTypeLabel;
        QLabel* mGaussStdLabel;
        QLabel* mRunTypeLabel;
        int mFilterType;
        int mFilterSize;
        float mGaussStdDev;
        int mRunType;

        std::string mFilterTypeString;
        std::string mRunTypeString;

        std::string mFilenameSetTo;

        QLabel* mExecuteTimeLabel;
        QLabel* mSetupTimeLabel;
        //QLabel* mExecuteTime2Label;
        QLabel* mSetupTime2Label;
        QLabel* mCreateMaskLabel;
        QLabel* mCreateMaskTwopassLabel;
        QLabel* mCreateMaskNaiveLabel;
        QLabel* mKernelTwopassLabel;
        QLabel* mKernelNaiveLabel;

        View* mView;
        View* mInitView;
        ImageFileImporter::pointer mImporter;
        ImageRenderer::pointer mInitRenderer;
        ProcessObjectPort mOutPort;
};

} // end namespace fast

#endif
