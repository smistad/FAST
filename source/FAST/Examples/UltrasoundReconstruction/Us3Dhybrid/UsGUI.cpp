/**
 * Examples/GUI/SimpleGUI/SimpleGUI.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "UsGUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/MeshRenderer/MeshRenderer.hpp>
#include <boost/bind.hpp>
#include "FAST/TestDataPath.hpp"

namespace fast {

    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n = 6)
    {
        std::ostringstream out;
        out << std::setprecision(n) << a_value;
        return out.str();
    }

    bool UsGUI::setInputNr(int inputNr){
        switch (inputNr){
        case 0:
            mInputFolder = "/rekonstruksjons_data/US_01_20130529T084519/";
            mInputFormat = "US_01_20130529T084519_ScanConverted_#.mhd";
            mZspacing = 0.1f;
            mDV = 1.0f;
            mRmax = mDV * 10.0f;
            break;
        case 1:
            mInputFolder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_01_19700101T102623/";
            mInputFormat = "US-Acq_01_19700101T102623_Tissue_#.mhd";
            mZspacing = 0.3f; 
            mDV = 1.0f; 
            mRmax = mDV * 15.0f;
            break;
        case 2:
            mInputFolder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_03_19700101T103031/";
            mInputFormat = "US-Acq_03_19700101T103031_Tissue_#.mhd";
            mZspacing = 0.2f;
            mDV = 1.0f;
            mRmax = mDV * 8.0f;
            break;
        default:
            return false;
        }
        std::string text = "Input set: " + mInputFormat;
        mInputLabel->setText(text.c_str());
        return true;
    }

    void UsGUI::setRunType(int runMode){
        mRunType = (Us3DRunMode)runMode;
        std::string text = "Run mode: ";// +std::to_string((Us3DRunMode)runMode);
        switch ((Us3DRunMode)runMode){
        case Us3DRunMode::clHybrid:
            text += "clHybrid";
            break;
        case Us3DRunMode::cpuHybrid:
            text += "clHybrid";
            break;
        case Us3DRunMode::clVNN:
            text += "clHybrid";
            break;
        case Us3DRunMode::cpuVNN:
            text += "clHybrid";
            break;
        case Us3DRunMode::clPNN:
            text += "clHybrid";
            break;
        case Us3DRunMode::cpuPNN:
            text += "clHybrid";
            break;
        default:
            text += "-";
        }
        mRunTypeLabel->setText(text.c_str());
    }

    void UsGUI::runAlgorithmAndExportImage(
        float setDV, float maxRvalue,
        std::string input_filename, std::string nameformat, std::string output_subfolder,
        int volSizeM, float initZspacing,
        Us3DRunMode runType,
        int startNumber, int stepSize
        ){

        if (runType == Us3DRunMode::clVNN || runType == Us3DRunMode::clPNN){
            return; //Unimplemented runType
        }
        std::cout << "## RUNNING with settings - setDV: " << setDV << " & rMax: " << maxRvalue << " ##" << std::endl;
        ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
        {
            streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
            streamer->setFilenameFormat(input_filename);
            streamer->setStartNumber(startNumber);
            streamer->setStepSize(stepSize);
        }

        std::string output_filename = ""; {
            // Create directory if does not exist
            std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "/" + output_subfolder;
            std::string streamStart = std::to_string(startNumber);
            std::string streamStep = std::to_string(stepSize);
            std::string volumeDV = to_string_with_precision(setDV, 3);
            std::string volumeRmax = to_string_with_precision(maxRvalue, 3);
            std::string volumeSizeMillion = std::to_string(volSizeM);
            std::string volumeZinitSpacing = to_string_with_precision(initZspacing, 3);
            std::string runningStyle = "";
            switch (runType){
            case Us3DRunMode::clHybrid:
                runningStyle += "CL_";
                break;
            case Us3DRunMode::cpuHybrid:
                break;
            case Us3DRunMode::cpuVNN:
                runningStyle += "VNN_";
                break;
            case Us3DRunMode::cpuPNN:
                runningStyle += "PNN_";
                break;
            default:
                std::cout << "Run type " << (Us3DRunMode)runType << " is not implemented. Quitting.." << std::endl;
                return;
            }

            output_filename += _filePath + "VOLUME_" + runningStyle + volumeSizeMillion + "M_" + "start-" + streamStart + "@" + streamStep;
            output_filename += "(dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd";
            std::cout << "Output filename: " << output_filename << std::endl;
        }

        Us3Dhybrid::pointer pnnHybrid;
        {
            // Reconstruction PNN
            pnnHybrid = Us3Dhybrid::New();
            pnnHybrid->setInputConnection(streamer->getOutputPort());
            pnnHybrid->setDV(setDV);
            pnnHybrid->setRmax(maxRvalue);
            pnnHybrid->setVolumeSize(volSizeM);
            pnnHybrid->setZDirInitSpacing(initZspacing);
            //Priority VNN > PNN > CL > Normal
            pnnHybrid->setRunMode(runType);

            while (!pnnHybrid->hasCalculatedVolume()){
                pnnHybrid->update();
            }
        }
    }

    UsGUI::UsGUI(){
        int SLIDER_WIDTH = 400;
        int inputSliderMax = 3;
        int runTypeSliderMax = 5;

        mInputNr = 0;
        mOutputSubFolder = "";
        mRunType = Us3DRunMode::clHybrid;
        mDV = 1.0f;
        mRmax = 1.0f;
        mZspacing = 0.1f;
        //bool mCalcZspacing; //avg/max of two others //TODO?

        /*
        // Create a 3D view
        View* view = createView();
        view->set3DMode();

        enableFullscreen();
        */

        // First create the menu layout
        QVBoxLayout* menuLayout = new QVBoxLayout;
        {
            // Menu items should be aligned to the top
            menuLayout->setAlignment(Qt::AlignTop);

            // Title label
            QLabel* title = new QLabel;
            title->setText("Menu");
            QFont font;
            font.setPointSize(28);
            title->setFont(font);
            menuLayout->addWidget(title);

            // Quit button
            QPushButton* quitButton = new QPushButton;
            quitButton->setText("Quit");
            quitButton->setFixedWidth(200);
            menuLayout->addWidget(quitButton);

            // Connect the clicked signal of the quit button to the stop method for the window
            QObject::connect(quitButton, &QPushButton::clicked, boost::bind(&Window::stop, this));
        }
        

        // LABELS AND SLIDERS
        mInputLabel = new QLabel;
        {
            mInputLabel->setText("Input set: -");
            menuLayout->addWidget(mInputLabel);
            setInputNr(mInputNr);

            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(inputSliderMax);
            slider->setValue(mInputNr);
            slider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(slider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(slider, &QSlider::valueChanged, boost::bind(&UsGUI::setInputNr, this, _1));
        }

        mRunTypeLabel = new QLabel;
        {
            mRunTypeLabel->setText("Run type: -");
            menuLayout->addWidget(mRunTypeLabel);
            setRunType(mRunType);

            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(runTypeSliderMax);
            slider->setValue(mRunType);
            slider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(slider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(slider, &QSlider::valueChanged, boost::bind(&UsGUI::setRunType, this, _1));
        }

        //Labels TODO
        //DV
        //RMAX
        //ZSpacing
        //textfield for outputSubfolder?
        //ADD button to RUN (check out exit button etc for ideas)
        //TODO maybe add view again?



        // Add menu and view to main layout
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addLayout(menuLayout);
        //layout->addWidget(view);

        mWidget->setLayout(layout);
    }


    /*
    // Update label
        std::string text = "Smoothing: " + boost::lexical_cast<std::string>(standardDeviation)+" mm";
        mSmoothingLabel->setText(text.c_str());
    */

}
