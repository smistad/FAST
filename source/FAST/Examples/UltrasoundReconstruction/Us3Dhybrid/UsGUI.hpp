/**
 * Examples/GUI/SimpleGUI/SimpleGUI.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#ifndef SIMPLE_GUI_HPP_
#define SIMPLE_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include <QLabel>

#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/TestDataPath.hpp"
#include "FAST/Algorithms/UsReconstruction/Us3Dhybrid/Us3Dhybrid.hpp"

namespace fast {

    class UsGUI : public Window {
        FAST_OBJECT(UsGUI)
    public:
        void runAlgorithmAndExportImage(
            float setDV, float maxRvalue,
            std::string input_filename, std::string nameformat, std::string output_subfolder = "",
            int volSizeM = 32, float initZspacing = 1.0f,
            Us3DRunMode runType = Us3DRunMode::clHybrid,
            int startNumber = 0, int stepSize = 1
            );
    private:
        UsGUI();

        bool setInputNr(int inputNr);
        void setRunType(int runMode); //Us3DRunMode

        //Us3Dhybrid::pointer mReconstruction;
        int mInputNr;
        std::string mInputFormat;
        std::string mInputFolder;
        std::string mOutputSubFolder;
        Us3DRunMode mRunType;
        float mDV;
        float mRmax;
        float mZspacing;
        //bool mCalcZspacing; //avg/max of two others

        QLabel* mInputLabel;
        QLabel* mRunTypeLabel;

        //std::string mInputLabelText; ??
    };
} // end namespace fast

#endif
