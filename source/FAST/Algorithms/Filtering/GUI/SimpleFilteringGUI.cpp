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
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <boost/bind.hpp>
#include "FAST/TestDataPath.hpp"

#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"

#include "FAST/Exporters/ImageExporter.hpp"

#include <chrono>
#include <thread>

namespace fast {

SimpleFilteringGUI::SimpleFilteringGUI() {

    int initialMaskSize = 9;
    float initialStdDev = 2.0;
    int initialRunType = 4; // 0:naive, 1:Twopass, 2:Local-Naive, 3:?, 4:Local-Twopass
    int initialFilterType = 1; // 1:Gauss, 2:Sobel, ..

    int initialInputImage = 1; // 0:US, 1:Retina(Big) 2:CornerTest, 3:Retina, 4:CornerTestMini, 5:Test(white)
    
    mFilterSize = initialMaskSize;
    mRunType = initialRunType;
    mRunTypeString = numToRunType(initialRunType);
    mGaussStdDev = initialStdDev;
    mSkipSave = false;//true;
    mSleepTime_maskChange = 8000;

	// Create a 2D view
    mView = createView();
    mView->set2DMode();
    mView->setMaximumFramerate(1);
    mView->setFixedWidth(600);

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
    mSobelX->enableRuntimeMeasurements(); //for timing

    //SobelFiltering::pointer 
    mSobelY = SobelFiltering::New();
    mSobelY->setDirection(1); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    mSobelY->setInputConnection(mImporter->getOutputPort());
    mSobelY->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive
    mSobelY->enableRuntimeMeasurements(); //for timing

    //AverageImages::pointer 
    mSobelTot = AverageImages::New();
    mSobelTot->setCutOverhead(true); //true
    mSobelTot->setInputConnection(0, mSobelX->getOutputPort());
    mSobelTot->setInputConnection(1, mSobelY->getOutputPort());
    mSobelTot->enableRuntimeMeasurements(); //for timing

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
        mFilterType = 2;
        break;
    default:
        mFilterTypeString = "Avg";
        mOutPort = mBoxFilter->getOutputPort();
        mFilterType = 0;
    }
    renderer->addInputConnection(mOutPort);
    
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
    // Input image parameter slider
    QSlider* inputImageSlider = new QSlider(Qt::Horizontal);
    inputImageSlider->setMinimum(0);
    inputImageSlider->setMaximum(5);
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
    gaussStdDevSlider->setMaximum(4.0);
    gaussStdDevSlider->setValue((initialStdDev + 2 )/ 3);
    gaussStdDevSlider->setFixedWidth(200);
    menuLayout->addWidget(gaussStdDevSlider);
    // Connect the value changed signal of the slider to the updateGaussStd method
    QObject::connect(gaussStdDevSlider, &QSlider::valueChanged, boost::bind(&SimpleFilteringGUI::updateGaussStd, this, _1));

    // RunType parameter label 
    mRunTypeLabel = new QLabel;
    std::string runLabelText = "Run type: " + mRunTypeString;
    mRunTypeLabel->setText( runLabelText.c_str());
    menuLayout->addWidget(mRunTypeLabel);
    // RunType parameter slider
    QSlider* runTypeSlider = new QSlider(Qt::Horizontal);
    runTypeSlider->setMinimum(0);
    runTypeSlider->setMaximum(4);
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
    // SetupTime2 parameter label 
    mSetupTime2Label = new QLabel;
    mSetupTime2Label->setText("Setup naive time: -- ms");
    menuLayout->addWidget(mSetupTime2Label);
    // SetupTime Local parameter label 
    mSetupTimeLocalLabel = new QLabel;
    mSetupTimeLocalLabel->setText("Setup local time: -- ms");
    menuLayout->addWidget(mSetupTimeLocalLabel);
    // SetupTime Local Twopass parameter label
    mSetupTimeLocalTwoLabel = new QLabel;
    mSetupTimeLocalTwoLabel->setText("Setup local twopass time: -- ms");
    menuLayout->addWidget(mSetupTimeLocalTwoLabel);
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
    // Kernel twopass parameter label 
    mKernelTwopassLabel = new QLabel;
    mKernelTwopassLabel->setText("Kernel (twopass) time: -- ms");
    menuLayout->addWidget(mKernelTwopassLabel);
    // Kernel naive parameter label 
    mKernelNaiveLabel = new QLabel;
    mKernelNaiveLabel->setText("Kernel (naive) time: -- ms");
    menuLayout->addWidget(mKernelNaiveLabel);
    // Kernel local naive parameter label 
    mKernelLocalLabel = new QLabel;
    mKernelLocalLabel->setText("Kernel (local) time: -- ms");
    menuLayout->addWidget(mKernelLocalLabel);
    // Kernel local twopass  parameter label 
    mKernelLocalTwoLabel = new QLabel;
    mKernelLocalTwoLabel->setText("Kernel (local-2P) time: -- ms");
    menuLayout->addWidget(mKernelLocalTwoLabel);

    // Force print button
    QPushButton* printButton = new QPushButton;
    printButton->setText("Print stats");
    printButton->setFixedWidth(200);
    menuLayout->addWidget(printButton);
    // Connect the clicked signal of the quit button to the stop method for the window
    QObject::connect(printButton, &QPushButton::clicked, [=]{
        SimpleFilteringGUI::updateRuntimes(mGaussian, true);
    });

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(mInitView);
    layout->addWidget(mView);

    mWidget->setLayout(layout);
    
    saveImage();
}

std::string SimpleFilteringGUI::getInputFilename(int inputnum){
    std::string INPUT_FILENAME;
    switch (inputnum){
    case 1:
        INPUT_FILENAME = "retina_big.png"; 
        break;
    case 2:
        INPUT_FILENAME = "cornerTest.png"; 
        break;
    case 3:
        INPUT_FILENAME = "retina.png";
        break;
    case 4:
        INPUT_FILENAME = "cornerTestMini.png";
        break;
    case 5:
        INPUT_FILENAME = "test.png"; 
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
        return "Local-Naive";
    case 3:
        return "Local-Def";
    case 4:
        return "Local-Twopass";
    }

    return "None";

}

void SimpleFilteringGUI::updateRuntimes(Filtering::pointer filter, bool print){
    filter->update();
    if (print){
        filter->getRuntime()->print();
        filter->getRuntime("twopass_setup")->print();
        filter->getRuntime("naive_setup")->print();
        filter->getRuntime("local_setup")->print();
        filter->getRuntime("local_twopass_setup")->print();
        filter->getRuntime("create_twopass_mask")->print();
        filter->getRuntime("create_naive_mask")->print();
        filter->getRuntime("twopass_cl")->print();
        filter->getRuntime("naive_cl")->print();
        filter->getRuntime("local_cl")->print();
        filter->getRuntime("local_twopass_cl")->print();
    }
    //getLast() vs getSlidingAverage()
    float timingLast_tot = filter->getRuntime()->getSlidingAverage();
    float timingLast_twopass_setup = filter->getRuntime("twopass_setup")->getSlidingAverage();
    float timingLast_naive_setup = filter->getRuntime("naive_setup")->getSlidingAverage();
    float timingLast_local_setup = filter->getRuntime("local_setup")->getSlidingAverage();
    float timingLast_local_twopass_setup = filter->getRuntime("local_twopass_setup")->getSlidingAverage();

    float timingLast_create_mask = filter->getRuntime("create_mask")->getSlidingAverage();
    float timingLast_create_twopass_mask = filter->getRuntime("create_twopass_mask")->getSlidingAverage();
    float timingLast_create_naive_mask = filter->getRuntime("create_naive_mask")->getSlidingAverage();

    float timingLast_twopass_kernel = filter->getRuntime("twopass_cl")->getSlidingAverage();
    float timingLast_naive_kernel = filter->getRuntime("naive_cl")->getSlidingAverage();
    float timingLast_local_kernel = filter->getRuntime("local_cl")->getSlidingAverage();
    float timingLast_local_twopass_kernel = filter->getRuntime("local_twopass_cl")->getSlidingAverage();

    std::string executeText = "Execute time: " + std::to_string(timingLast_tot) + " ms";
    mExecuteTimeLabel->setText(executeText.c_str());
    std::string setupTwopassText = "Setup twopass time: " + std::to_string(timingLast_twopass_setup) + " ms";
    mSetupTimeLabel->setText(setupTwopassText.c_str());
    std::string setupNaiveText = "Setup naive time: " + std::to_string(timingLast_naive_setup) + " ms";
    mSetupTime2Label->setText(setupNaiveText.c_str());
    std::string setupLocalText = "Setup local time: " + std::to_string(timingLast_local_setup) + " ms";
    mSetupTimeLocalLabel->setText(setupLocalText.c_str());
    std::string setupLocalTwoText = "Setup local twopass time: " + std::to_string(timingLast_local_twopass_setup) + " ms";
    mSetupTimeLocalTwoLabel->setText(setupLocalTwoText.c_str());

    std::string createMaskText = "Create mask time: " + std::to_string(timingLast_create_mask) + " ms";
    mCreateMaskLabel->setText(createMaskText.c_str());
    std::string createMaskTwopassText = "Create mask (twopass) time: " + std::to_string(timingLast_create_twopass_mask) + " ms";
    mCreateMaskTwopassLabel->setText(createMaskTwopassText.c_str());
    std::string createMaskNaiveText = "Create mask (naive) time: " + std::to_string(timingLast_create_naive_mask) + " ms";
    mCreateMaskNaiveLabel->setText(createMaskNaiveText.c_str());

    std::string kernelTwopassText = "Kernel (twopass) time: " + std::to_string(timingLast_twopass_kernel) + " ms";
    mKernelTwopassLabel->setText(kernelTwopassText.c_str());
    std::string kernelNaiveText = "Kernel (naive) time: " + std::to_string(timingLast_naive_kernel) + " ms";
    mKernelNaiveLabel->setText(kernelNaiveText.c_str());
    std::string kernelLocalText = "Kernel (local) time: " + std::to_string(timingLast_local_kernel) + " ms";
    mKernelLocalLabel->setText(kernelLocalText.c_str());
    std::string kernelLocalTwoText = "Kernel (local-2P) time: " + std::to_string(timingLast_local_twopass_kernel) + " ms";
    mKernelLocalTwoLabel->setText(kernelLocalTwoText.c_str());
}

//// -- Functions updating values upon slider change -- ////
void SimpleFilteringGUI::updateInputImage(int value){
    std::string newFilename = getInputFilename(value);
    if (newFilename == mFilenameSetTo) return;
    mFilenameSetTo = newFilename;
    mImporter->setFilename(std::string(FAST_TEST_DATA_DIR) + newFilename);
    ProcessObjectPort port = mImporter->getOutputPort();
    
    stopComputationThread();
    //Fixed by initializing new renderer
    mGaussian->setInputConnection(port);
    mSobelX->setInputConnection(port);
    mSobelY->setInputConnection(port);
    startComputationThread();
   
    std::string text = "Input image: " + newFilename;
    mInputImageLabel->setText(text.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    saveImage();   
}
void SimpleFilteringGUI::updateFilterSize(int value){
    mFilterSize = value * 2 + 1;
    mGaussian->setMaskSize(mFilterSize);
    //Sobel not to be updated

    std::string text = "Filter size: " + boost::lexical_cast<std::string>(mFilterSize);
    mFilterSizeLabel->setText(text.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime_maskChange)); //8000 15000));
    if (mFilterType != 2) saveImage(); // if not sobel
}

void SimpleFilteringGUI::updateFilterType(int value){
    if (value == mFilterType) return;
    ImageRenderer::pointer newRenderer = ImageRenderer::New();
    switch (value){
    case 1:
        mFilterTypeString = "Gauss";
        mOutPort = mGaussian->getOutputPort();
        mFilterType = 1;
        break;
    case 2:
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

    std::string text = "Filter type: " + mFilterTypeString;
    mFilterTypeLabel->setText(text.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(180));
    saveImage();
}

void SimpleFilteringGUI::updateGaussStd(float value){

    mGaussStdDev = value*3-2;
    mGaussian->setStandardDeviation(mGaussStdDev);

    std::string text = "Std dev (gauss): " + boost::lexical_cast<std::string>(mGaussStdDev);
    mGaussStdLabel->setText(text.c_str());
    if(mFilterType==1) saveImage(); //only if gaussian

    if (mFilterType == 1){
        updateRuntimes(mGaussian); 
    }
    else if (mFilterType == 2){
        updateRuntimes(mSobelX);
    }
}

void SimpleFilteringGUI::updateRunType(int value){
    if (value < 0 || value > 4 || value==mRunType) return;

    mGaussian->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    mSobelX->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    mSobelY->setConvRunType(value); // 1:twopass, 2:adv, else: naive
    //mBoxFilter->setConvRunType(value); // 1:twopass, 2:adv, else: naive

    mRunType = value;
    mRunTypeString = numToRunType(value);
    std::string text = "Run type: " + mRunTypeString;
    mRunTypeLabel->setText(text.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));// 180));
    saveImage();
}


void SimpleFilteringGUI::saveImage(){
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); //20
    if (mSkipSave) return;
    
    // Exporter image
    ImageExporter::pointer exporter = ImageExporter::New();
    std::string configString;
    if (mFilterTypeString == "Sobel"){
        configString = mFilterTypeString + "-" + mRunTypeString;
    }
    else {
        configString = "";
        if (mFilterTypeString == "Gauss") configString += std::to_string(mGaussStdDev) + "-";
        configString += mFilterTypeString + "-" + std::to_string(mFilterSize) + "-" + mRunTypeString;
        
    }
    //add a timestamp?
    std::string output_filename = mFilenameSetTo + "_"+ configString +"_out.png";
    std::string sub_folders = "/output/GUI/"+mFilenameSetTo + "/" + mFilterTypeString + "/" + mRunTypeString + "/";
    std::string output_filepath = std::string(FAST_TEST_DATA_DIR) + sub_folders + output_filename;
    exporter->setFilename(output_filepath);
    exporter->setInputConnection(mOutPort);
    exporter->update();
    std::cout << "Saved imaged '" << output_filename << "' to '" << sub_folders << "'!" << std::endl;
}
}
