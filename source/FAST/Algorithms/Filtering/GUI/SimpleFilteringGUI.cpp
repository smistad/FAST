/**
 * Examples/GUI/SimpleGUI/SimpleGUI.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
//#include "SimpleGUI.hpp"
#include "SimpleFilteringGUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <FAST/Importers/ImageFileImporter.hpp>
//#include <FAST/Visualization/MeshRenderer/MeshRenderer.hpp>
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <boost/bind.hpp>
#include "FAST/TestDataPath.hpp"

#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"

#include "FAST/Exporters/ImageExporter.hpp"
//#include <unistd.h>
#include <chrono>
#include <thread>

namespace fast {

SimpleFilteringGUI::SimpleFilteringGUI() {

    int initialMaskSize = 7; 
    float initialStdDev = 1.0; 
    int initialRunType = 1; // 0:naive, 1:Twopass
    int initialFilterType = 2; // 1:Gauss, 2:Sobel, ..

    int initialInputImage = 1; // 0:US, 1:Retina 2:CornerTest

    //
    mFilterSize = initialMaskSize;
    mRunType = initialRunType;
    mRunTypeString = numToRunType(initialRunType);
    mGaussStdDev = initialStdDev;

	// Create a 2D view
    mView = createView();
    mView->set2DMode();
    mView->setMaximumFramerate(1);
    mView->setFixedWidth(600);
    //enableFullscreen();

    // Import image
    mImporter = ImageFileImporter::New();
    std::string initialInputImageName = getInputFilename(initialInputImage);
    mFilenameSetTo = initialInputImageName;
    mImporter->setFilename(std::string(FAST_TEST_DATA_DIR) + initialInputImageName);

    //// --  Initialize Filters -- ////
    // Define Filtering (box)
    mBoxFilter = Filtering::New();

    // Define Gaussian
    mGaussian = GaussianFiltering::New();
    mGaussian->setStandardDeviation(initialStdDev);
    mGaussian->setMaskSize(initialMaskSize);
    mGaussian->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive
    mGaussian->setInputConnection(mImporter->getOutputPort());
    mGaussian->enableRuntimeMeasurements(); //for timing

    // Define Sobel
    //SobelFiltering::pointer 
    mSobelX = SobelFiltering::New();
    mSobelX->setDirection(0); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    mSobelX->setInputConnection(mImporter->getOutputPort());
    mSobelX->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive

    //SobelFiltering::pointer 
    mSobelY = SobelFiltering::New();
    mSobelY->setDirection(1); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    mSobelY->setInputConnection(mImporter->getOutputPort());
    mSobelY->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive

    //AverageImages::pointer 
    mSobelTot = AverageImages::New();
    mSobelTot->setCutOverhead(true);
    mSobelTot->setInputConnection(0, mSobelX->getOutputPort());
    mSobelTot->setInputConnection(1, mSobelY->getOutputPort());

    // Set up rendering
    ImageRenderer::pointer renderer = ImageRenderer::New();
    //std::string filterType;
    switch (initialFilterType){
    case 1:
        mFilterTypeString = "Gauss";
        mOutPort = mGaussian->getOutputPort();
        mFilterType = 1;
        break;
    case 2:
        mFilterTypeString = "Sobel";
        mOutPort = mSobelTot->getOutputPort();
        //renderer->addInputConnection(mSobelTot->getOutputPort());
        mFilterType = 2;
        break;
    default:
        mFilterTypeString = "Avg";
        mOutPort = mBoxFilter->getOutputPort();
        mFilterType = 0;
        //renderer->addInputConnection(mBoxFilter->getOutputPort());
    }
    renderer->addInputConnection(mOutPort);
    
    //renderer->addInputConnection(mGaussian->getOutputPort());
    //ImageRenderer::pointer 
    mInitRenderer = ImageRenderer::New();
    mInitRenderer->addInputConnection(mImporter->getOutputPort());
    mInitView = createView();
    mInitView->set2DMode();
    mInitView->setMaximumFramerate(1);
    mInitView->addRenderer(mInitRenderer);
    mInitView->setFixedWidth(600);


    // Add to view
    mView->addRenderer(renderer);
    
    //// --  Create and add GUI elements -- ////
    // First create the menu layout
    QVBoxLayout* menuLayout = new QVBoxLayout;
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

    //// -- Initializing labels and sliders -- ////
    // Input image parameter label
    mInputImageLabel = new QLabel;
    std::string inputImageLabelText = "Input image: " + initialInputImageName;
    mInputImageLabel->setText( inputImageLabelText.c_str() );
    menuLayout->addWidget(mInputImageLabel);
    // Filter size parameter slider
    QSlider* inputImageSlider = new QSlider(Qt::Horizontal);
    inputImageSlider->setMinimum(0);
    inputImageSlider->setMaximum(2);
    inputImageSlider->setValue(initialInputImage);
    inputImageSlider->setFixedWidth(200);
    menuLayout->addWidget(inputImageSlider);
    // Connect the value changed signal of the slider to the updateFilterSize method
    QObject::connect(inputImageSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateInputImage, this, _1));

    // Filter size parameter label
    mFilterSizeLabel = new QLabel; 
    std::string filterSizeText = "Filter size (non-sobel): " + boost::lexical_cast<std::string>(initialMaskSize);
    mFilterSizeLabel->setText(filterSizeText.c_str());
    menuLayout->addWidget(mFilterSizeLabel);
    // Filter size parameter slider
    QSlider* filterSizeSlider = new QSlider(Qt::Horizontal);
    filterSizeSlider->setMinimum(1);
    filterSizeSlider->setMaximum(7);
    filterSizeSlider->setValue( (mFilterSize-1)/2 );
    filterSizeSlider->setFixedWidth(200);
    menuLayout->addWidget(filterSizeSlider);
    // Connect the value changed signal of the slider to the updateFilterSize method
    QObject::connect(filterSizeSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateFilterSize, this, _1));

    // Filter type parameter label 
    mFilterTypeLabel = new QLabel;
    std::string filterTypeText = "Filter type: " + mFilterTypeString;
    mFilterTypeLabel->setText(filterTypeText.c_str());
    menuLayout->addWidget(mFilterTypeLabel);
    // Filter type parameter slider
    QSlider* filterTypeSlider = new QSlider(Qt::Horizontal);
    filterTypeSlider->setMinimum(0);
    filterTypeSlider->setMaximum(2);
    filterTypeSlider->setValue(initialFilterType);
    filterTypeSlider->setFixedWidth(200);
    menuLayout->addWidget(filterTypeSlider);
    // Connect the value changed signal of the slider to the updateFilterType method
    QObject::connect(filterTypeSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateFilterType, this, _1));

    // Gaussian std dev parameter label 
    mGaussStdLabel = new QLabel;
    std::string gaussStdDevText = "Std dev (gauss): " + boost::lexical_cast<std::string>(initialStdDev);
    mGaussStdLabel->setText( gaussStdDevText.c_str() );
    menuLayout->addWidget(mGaussStdLabel);
    // Gaussian std dev parameter slider
    QSlider* gaussStdDevSlider = new QSlider(Qt::Horizontal);
    gaussStdDevSlider->setMinimum(1.0);
    gaussStdDevSlider->setMaximum(80.0);
    gaussStdDevSlider->setValue(initialStdDev*4);
    gaussStdDevSlider->setFixedWidth(200);
    menuLayout->addWidget(gaussStdDevSlider);
    // Connect the value changed signal of the slider to the updateGaussStd method
    QObject::connect(gaussStdDevSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateGaussStd, this, _1));

    // RunType parameter label 
    mRunTypeLabel = new QLabel;
    std::string runLabelText = "Run type: " + mRunTypeString;
    mRunTypeLabel->setText( runLabelText.c_str()); // "Run type: ---"
    menuLayout->addWidget(mRunTypeLabel);
    // RunType parameter slider
    QSlider* runTypeSlider = new QSlider(Qt::Horizontal);
    runTypeSlider->setMinimum(0);
    runTypeSlider->setMaximum(2);
    runTypeSlider->setValue(initialRunType);
    runTypeSlider->setFixedWidth(200);
    menuLayout->addWidget(runTypeSlider);
    // Connect the value changed signal of the slider to the updateRunType method
    QObject::connect(runTypeSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateRunType, this, _1));

    // Display value labels
    QLabel* separator = new QLabel;
    separator->setText("\n\n");
    menuLayout->addWidget(separator);
    // ExecuteTime parameter label 
    mExecuteTimeLabel = new QLabel;
    mExecuteTimeLabel->setText("Execute time: -- ms");
    menuLayout->addWidget(mExecuteTimeLabel);
    // SetupTime parameter label 
    mSetupTimeLabel = new QLabel;
    mSetupTimeLabel->setText("Setup twopass time: -- ms");
    menuLayout->addWidget(mSetupTimeLabel);
    /*// ExecuteTime2 parameter label 
    mExecuteTime2Label = new QLabel;
    mExecuteTime2Label->setText("Execute2 time: --ms");
    menuLayout->addWidget(mExecuteTime2Label);*/
    // SetupTime2 parameter label 
    mSetupTime2Label = new QLabel;
    mSetupTime2Label->setText("Setup naive time: -- ms");
    menuLayout->addWidget(mSetupTime2Label);
    // CreateMask parameter label 
    mCreateMaskLabel = new QLabel;
    mCreateMaskLabel->setText("Create mask time: -- ms");
    menuLayout->addWidget(mCreateMaskLabel);
    // CreateMask twopass parameter label 
    mCreateMaskTwopassLabel = new QLabel;
    mCreateMaskTwopassLabel->setText("Create mask (twopass) time: -- ms");
    menuLayout->addWidget(mCreateMaskTwopassLabel);
    // CreateMask parameter label 
    mCreateMaskNaiveLabel = new QLabel;
    mCreateMaskNaiveLabel->setText("Create mask (naive) time: -- ms");
    menuLayout->addWidget(mCreateMaskNaiveLabel);

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(mInitView); // , 0, Qt::AlignCenter);
    layout->addWidget(mView); // , 0, Qt::AlignTop);

    mWidget->setLayout(layout);
    
    saveImage();
}

//// -- Helper functions num to some string -- ////
std::string SimpleFilteringGUI::getInputFilename(int inputnum){
    std::string INPUT_FILENAME;
    switch (inputnum){
    case 1:
        INPUT_FILENAME = "retina.png";
        break;
    case 2:
        INPUT_FILENAME = "cornerTest.png"; //cornerTest2.png
        break;
    default:
        INPUT_FILENAME = "US-2D.jpg";
    }
    return INPUT_FILENAME;
}

std::string SimpleFilteringGUI::numToRunType(int num){
    switch (num){
    case 0:
        return "Naive";
    case 1:
        return "Twopass";
    case 2:
        return "Advanced";
    }

    return "None";

}

//// -- Functions updating values upon slider change -- ////
void SimpleFilteringGUI::updateInputImage(int value){
    std::string newFilename = getInputFilename(value);
    if (newFilename == mFilenameSetTo) return;
    mFilenameSetTo = newFilename;
    mImporter = ImageFileImporter::New();
    mImporter->setFilename(std::string(FAST_TEST_DATA_DIR) + newFilename);
    ProcessObjectPort port = mImporter->getOutputPort();
    
    stopComputationThread();
    //Fixed by initializing new renderer
    mGaussian->setInputConnection(port);
    mSobelX->setInputConnection(port);
    mSobelY->setInputConnection(port);
    ImageRenderer::pointer newRenderer = ImageRenderer::New();
    newRenderer->addInputConnection(port);
    mInitView->removeAllRenderers();
    mInitView->addRenderer(newRenderer);
    mInitView->setFixedWidth(600);
    startComputationThread();
   
    std::string text = "Input image: " + newFilename;
    mInputImageLabel->setText(text.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    saveImage();
}
void SimpleFilteringGUI::updateFilterSize(int value){
    mFilterSize = value * 2 + 1;
    //int newMaskSize = value * 2 + 1;
    //mFilterSize = value * 2 + 1;
    mGaussian->setMaskSize(mFilterSize);
    //mBoxFilter->setMaskSize(mFilterSize);
    //Sobel not to be updated

    std::string text = "Filter size: " + boost::lexical_cast<std::string>(mFilterSize);
    mFilterSizeLabel->setText(text.c_str());
    if (mFilterType != 2) saveImage(); // if not sobel
}

void SimpleFilteringGUI::updateFilterType(int value){
    if (value == mFilterType) return;
    ImageRenderer::pointer newRenderer = ImageRenderer::New();
    //ProcessObjectPort port;
    switch (value){
    case 1:
        mFilterTypeString = "Gauss";
        mOutPort = mGaussian->getOutputPort();
        mFilterType = 1;
        break;
    case 2:
        //return;
        mFilterTypeString = "Sobel";
        mOutPort = mSobelTot->getOutputPort();
        mFilterType = 2;
        break;
    default:
        return;
        mFilterTypeString = "Avg";
        mFilterType = 0;
        mOutPort = mBoxFilter->getOutputPort();
    }

    newRenderer->addInputConnection(mOutPort);

    // Reset view to new renderer
    stopComputationThread();
    mView->removeAllRenderers();
    mView->addRenderer(newRenderer);
    mView->setFixedWidth(600);
    startComputationThread();

    std::string text = "Filter type: " + mFilterTypeString; //boost::lexical_cast<std::string>(value);
    mFilterTypeLabel->setText(text.c_str());
    saveImage();
}

void SimpleFilteringGUI::updateGaussStd(float value){
    mGaussStdDev = value / 4;
    //float newStdDev = value / 4;
    mGaussian->setStandardDeviation(mGaussStdDev);

    std::string text = "Std dev (gauss): " + boost::lexical_cast<std::string>(mGaussStdDev);
    mGaussStdLabel->setText(text.c_str());
    if(mFilterType==1) saveImage(); //only if gaussian
    if (mFilterType == 1){
        mGaussian->update();
        mGaussian->getRuntime()->print();
        /*float timingLast_tot = mGaussian->getRuntime()->getLast();
        mGaussian->getRuntime("twopass_setup")->print();
        float timingLast_twopass_setup = mGaussian->getRuntime("twopass_setup")->getLast();
        //mGaussian->getRuntime("twopass_cl")->print();
        mGaussian->getRuntime("naive_setup")->print();
        float timingLast_naive_setup = mGaussian->getRuntime("naive_setup")->getLast();
        mGaussian->getRuntime("create_mask")->print();
        float timingLast_create_mask = mGaussian->getRuntime("create_mask")->getLast();
        mGaussian->getRuntime("create_twopass_mask")->print();
        float timingLast_create_twopass_mask = mGaussian->getRuntime("create_twopass_mask")->getLast();
        mGaussian->getRuntime("create_naive_mask")->print();
        float timingLast_create_naive_mask = mGaussian->getRuntime("create_naive_mask")->getLast();
        //mGaussian->getRuntime("naive_cl")->print();
        std::string executeText = "Execute time: " + std::to_string(timingLast_tot)+" ms";
        mExecuteTimeLabel->setText(executeText.c_str());
        std::string setupTwopassText = "Setup twopass time: " + std::to_string(timingLast_twopass_setup) + " ms";
        mSetupTimeLabel->setText(setupTwopassText.c_str());
        std::string setupNaiveText = "Setup naive time: " + std::to_string(timingLast_naive_setup) + " ms";
        mSetupTime2Label->setText(setupNaiveText.c_str());
        std::string createMaskText = "Create mask time: " + std::to_string(timingLast_create_mask) + " ms";
        mCreateMaskLabel->setText(createMaskText.c_str());
        std::string createMaskTwopassText = "Create mask (twopass) time: " + std::to_string(timingLast_create_twopass_mask) + " ms";
        mCreateMaskTwopassLabel->setText(createMaskTwopassText.c_str());
        std::string createMaskNaiveText = "Create mask (naive) time: " + std::to_string(timingLast_create_naive_mask) + " ms";
        mCreateMaskNaiveLabel->setText(createMaskNaiveText.c_str());*/
    }
}

void SimpleFilteringGUI::updateRunType(int value){
    if (value < 0 || value >= 2 || value==mRunType) return;

    mGaussian->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    mSobelX->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    mSobelY->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    //mBoxFilter->setConvRunType(value); // 1:twopass, 2:adv, else: naive

    mRunType = value;
    mRunTypeString = numToRunType(value);
    std::string text = "Run type: " + mRunTypeString;
    mRunTypeLabel->setText(text.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(180));
    saveImage();
}


void SimpleFilteringGUI::saveImage(){
    
    std::this_thread::sleep_for( std::chrono::milliseconds(20) ); 
    // Exporter image
    ImageExporter::pointer exporter = ImageExporter::New();
    // add string with time.h (time(NULL)) etc
    //std::cout << "FAST_TEST_DATA_DIR" << FAST_TEST_DATA_DIR << std::endl;
    std::string configString;
    if (mFilterTypeString == "Sobel"){
        configString = mFilterTypeString + "-" + mRunTypeString;
    }
    else {
        configString = "";
        if (mFilterTypeString == "Gauss") configString += std::to_string(mGaussStdDev) + "-";
        configString += mFilterTypeString + "-" + std::to_string(mFilterSize) + "-" + mRunTypeString;
        
    }
    //std::string configString = mFilterTypeString+"-"+std::to_string(mFilterSize)+"-"+ mRunTypeString;
    
    //add a timestamp?
    std::string output_filename = mFilenameSetTo + "_"+ configString +"_out.png";
    std::string sub_folders = "/output/GUI/"+mFilenameSetTo + "/" + mFilterTypeString + "/";
    std::string output_filepath = std::string(FAST_TEST_DATA_DIR) + sub_folders + output_filename;
    exporter->setFilename(output_filepath);
    exporter->setInputConnection(mOutPort);
    exporter->update();
    std::cout << "Saved imaged '" << output_filename << "' to '" << sub_folders << "'!" << std::endl;
}

}
