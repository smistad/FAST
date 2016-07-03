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
#include <boost/bind.hpp>

#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/Algorithms/UsReconstruction/Us3Dhybrid/Us3Dhybrid.hpp"
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
        std::string text = mInputBaseLabel + mInputFormat;
        mInputLabel->setText(text.c_str());
        return true;
    }

    void UsGUI::setRunType(int runMode){
        mRunType = (Us3DRunMode)runMode;
        std::string text = mRunTypeBaseLabel;// +std::to_string((Us3DRunMode)runMode);
        switch ((Us3DRunMode)runMode){
        case Us3DRunMode::clHybrid:
            text += "clHybrid";
            break;
        case Us3DRunMode::cpuHybrid:
            text += "cpuHybrid";
            break;
        case Us3DRunMode::clVNN:
            text += "clVNN";
            break;
        case Us3DRunMode::cpuVNN:
            text += "cpuVNN";
            break;
        case Us3DRunMode::clPNN:
            text += "clPNN";
            break;
        case Us3DRunMode::cpuPNN:
            text += "cpuPNN";
            break;
        default:
            text += "-";
        }
        mRunTypeLabel->setText(text.c_str());
    }

    void UsGUI::setDV(float dv){
        mDV = dv;
        std::string text = mDvBaseLabel;
        text += boost::lexical_cast<std::string>(dv);
        mDvLabel->setText(text.c_str());
    }
    void UsGUI::setDVSlider(int dvInt){
        float dv = dvInt * dvGran;
        setDV(dv);
    }

    void UsGUI::setRmax(float rMax){
        mRmax = rMax;
        std::string text = mRmaxBaseLabel;
        text += boost::lexical_cast<std::string>(rMax);
        mRmaxLabel->setText(text.c_str());
    }
    void UsGUI::setRmaxSlider(int rMaxInt){
        float rMax = rMaxInt * rMaxGran;
        setRmax(rMax);
    }

    void UsGUI::setZspacing(float zSpac){
        mZspacing = zSpac;
        std::string text = mZspacingBaseLabel;
        text += boost::lexical_cast<std::string>(zSpac);
        mZspacingLabel->setText(text.c_str());
    }
    void UsGUI::setZspacingSlider(int zSpacInt){
        float zSpac = zSpacInt * zSpacingGran;
        setZspacing(zSpac);
    }

    void UsGUI::setVolSizeM(int volStep){
        int volSizeM;
        switch (volStep){
        case 0:
            volSizeM = 32;
            break;
        case 1:
            volSizeM = 64;
            break;
        case 2:
            volSizeM = 128;
            break;
        case 3:
            volSizeM = 256;
            break;
        }
        mVolSizeM = volSizeM;
        std::string text = mVolSizeMBaseLabel;
        text += boost::lexical_cast<std::string>(volSizeM);
        mVolSizeMLabel->setText(text.c_str());
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

        MetaImageExporter::pointer exporter;
        {
            exporter = MetaImageExporter::New();
            exporter->setFilename(output_filename);
            exporter->enableCompression();
            Image::pointer resultVolume = pnnHybrid->getStaticOutputData<Image>(0);

            exporter->setInputData(resultVolume);
            exporter->update();
            std::cout << "Output filename: " << output_filename << std::endl;
        }
    }

    void UsGUI::runAlgorithmAndExportImage(){
        //calc input_filename?
        std::string input_filename = std::string(FAST_TEST_DATA_DIR) + mInputFolder + mInputFormat;
        runAlgorithmAndExportImage(
            mDV, mRmax,
            input_filename, mInputFormat, "",
            mVolSizeM, mZspacing, mRunType,
            mStreamStart, mStreamStep
            );
    }

    UsGUI::UsGUI(){
        int SLIDER_WIDTH = 400;
        {
            mInputBaseLabel = "Input set: ";
            mRunTypeBaseLabel = "Run type: ";
            mDvBaseLabel = "DV: ";
            mRmaxBaseLabel = "Rmax: ";
            mZspacingBaseLabel = "Init Z-spacing: ";
            mVolSizeMBaseLabel = "VolSize(M): ";
        }
        int inputSliderMax = 3;
        int runTypeSliderMax = 5;
        int volSizeMMax = 256; //32 /64/128/256
        {
            dvMax = 5.0f;
            rMaxMax = 30.0f; //eller change to multiplier?
            zSpacingMax = 1.0f;

            dvGran = 0.1f; //Granularity
            rMaxGran = 0.5f; //Granularity 0.2/0.5?
            zSpacingGran = 0.05f;
        }
        int dvSliderMax = ceil(dvMax / dvGran);
        int rMaxSliderMax = ceil(rMaxMax / rMaxGran);
        int zSpacingSliderMax = ceil(zSpacingMax / zSpacingGran);
        int volSizeMSliderMax = 3;
        
        {
            mInputNr = 0;
            mOutputSubFolder = "";
            mRunType = Us3DRunMode::clHybrid;
            mDV = 1.0f;
            mRmax = 1.0f;
            mZspacing = 0.1f;
            mVolSizeM = 128; //=2
            mVolSizeMController = 2;
            //bool mCalcZspacing; //avg/max of two others //TODO?
            mStreamStart = 0;
            mStreamStep = 1;
        }
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
            mInputLabel->setText(mInputBaseLabel.c_str());
            menuLayout->addWidget(mInputLabel);
            setInputNr(mInputNr);

            mInputSlider = new QSlider(Qt::Horizontal);
            mInputSlider->setMinimum(0);
            mInputSlider->setMaximum(inputSliderMax);
            mInputSlider->setValue(mInputNr);
            mInputSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mInputSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mInputSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setInputNr, this, _1));
        }
        mRunTypeLabel = new QLabel;
        {
            mRunTypeLabel->setText(mRunTypeBaseLabel.c_str());
            menuLayout->addWidget(mRunTypeLabel);
            setRunType(mRunType);

            mRunTypeSlider = new QSlider(Qt::Horizontal);
            mRunTypeSlider->setMinimum(0);
            mRunTypeSlider->setMaximum(runTypeSliderMax);
            mRunTypeSlider->setValue(mRunType);
            mRunTypeSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mRunTypeSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mRunTypeSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setRunType, this, _1));
        }
        mDvLabel = new QLabel;
        {
            mDvLabel->setText(mDvBaseLabel.c_str());
            menuLayout->addWidget(mDvLabel);
            setDV(mDV);

            mDvSlider = new QSlider(Qt::Horizontal);
            mDvSlider->setMinimum(0);
            mDvSlider->setMaximum(dvSliderMax);
            mDvSlider->setValue( (mDV/dvGran) );
            mDvSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mDvSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mDvSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setDVSlider, this, _1));
        }
        mRmaxLabel = new QLabel;
        {
            mRmaxLabel->setText(mRmaxBaseLabel.c_str());
            menuLayout->addWidget(mRmaxLabel);
            setRmax(mRmax);

            mRmaxSlider = new QSlider(Qt::Horizontal);
            mRmaxSlider->setMinimum(0);
            mRmaxSlider->setMaximum(rMaxSliderMax);
            mRmaxSlider->setValue((mRmax / rMaxGran));
            mRmaxSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mRmaxSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mRmaxSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setRmaxSlider, this, _1));
        }
        mZspacingLabel = new QLabel;
        {
            mZspacingLabel->setText(mZspacingBaseLabel.c_str());
            menuLayout->addWidget(mZspacingLabel);
            setZspacing(mZspacing);

            mZspacingSlider = new QSlider(Qt::Horizontal);
            mZspacingSlider->setMinimum(0);
            mZspacingSlider->setMaximum(zSpacingSliderMax);
            mZspacingSlider->setValue((mZspacing / zSpacingGran));
            mZspacingSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mZspacingSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mZspacingSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setZspacingSlider, this, _1));
        }
        mVolSizeMLabel = new QLabel;
        {
            mVolSizeMLabel->setText(mVolSizeMBaseLabel.c_str());
            menuLayout->addWidget(mVolSizeMLabel);
            //setRmax(mRmax);
            setVolSizeM(mVolSizeMController);

            mVolSizeMSlider = new QSlider(Qt::Horizontal);
            mVolSizeMSlider->setMinimum(0);
            mVolSizeMSlider->setMaximum(volSizeMSliderMax);
            mVolSizeMSlider->setValue(mVolSizeMController);
            mVolSizeMSlider->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(mVolSizeMSlider);

            // Connect the value changed signal of the slider to the setInputNr method
            QObject::connect(mVolSizeMSlider, &QSlider::valueChanged, boost::bind(&UsGUI::setVolSizeM, this, _1));
        }
        
        // RUN button
        QPushButton* runButton = new QPushButton;
        {
            runButton->setText("Run");
            runButton->setFixedWidth(SLIDER_WIDTH);
            menuLayout->addWidget(runButton);
            // Connect the clicked signal of the quit button to the stop method for the window
            QObject::connect(runButton, &QPushButton::clicked, boost::bind(&UsGUI::runAlgorithmAndExportImage, this));
        }

        //textfield for outputSubfolder?
        //TODO maybe add view again?

        // Add menu and view to main layout
        QHBoxLayout* layout = new QHBoxLayout;
        layout->addLayout(menuLayout);
        //layout->addWidget(view);

        mWidget->setLayout(layout);
    }

}
