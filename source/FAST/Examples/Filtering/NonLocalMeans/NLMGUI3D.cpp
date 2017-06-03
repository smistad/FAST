/**
 * Examples/GUI/SimpleGUI/SimpleGUI.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "NLMGUI3D.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp>
#include "FAST/Exporters/ImageExporter.hpp"
//#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <functional>


namespace fast {

NLMGUI3D::NLMGUI3D() {

    //viewOrig = createView();
    //viewOrig->set3DMode();
    //viewOrig->set3DMode();
    // Create a 3D view
    //View* view = createView();
    view = createView();
    view->set3DMode();
    //view->set2DMode();
    enableFullscreen();

    // Import image
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/US/Ball/US-3Dt_0.mhd");
    
    
    // Smooth images
    
    nlmSmoothing = NonLocalMeans::New();
    nlmSmoothing->setInputConnection(importer->getOutputPort());
    nlmSmoothing->setSigma(0.65f);
    nlmSmoothing->setGroupSize(3);
    nlmSmoothing->setWindowSize(3);
    nlmSmoothing->setDenoiseStrength(0.150f);
    nlmSmoothing->setK(0);
    //nlmSmoothing->enableRuntimeMeasurements();
    
    // Set up surface extraction
    mSurfaceExtraction = SurfaceExtraction::New();
    //mSurfaceExtraction->setInputConnection(importer->getOutputPort());
    mSurfaceExtraction->setInputConnection(nlmSmoothing->getOutputPort());
    mSurfaceExtraction->setThreshold(50);

    // Set up rendering
    TriangleRenderer::pointer renderer = TriangleRenderer::New();
    renderer->addInputConnection(mSurfaceExtraction->getOutputPort());
    view->addRenderer(renderer);
    
    // Create and add GUI elements
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
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));

    // Smoothing parameter label
    //mSmoothingLabel = new QLabel;
    //mSmoothingLabel->setText("DenoiseStrength: 300");
    //menuLayout->addWidget(mSmoothingLabel);
    nlmStrengthLabel = new QLabel;
    std::string text1 = "Denoise Strength: " + std::to_string(0.3) + " ";
    nlmStrengthLabel->setText(text1.c_str());
    menuLayout->addWidget(nlmStrengthLabel);
    // Smoothing parameter slider
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(10);
    slider->setMaximum(1000);
    slider->setValue(300);
    slider->setFixedWidth(300);
    menuLayout->addWidget(slider);

    // Connect the value changed signal of the slider to the updateSmoothingParameter method
    QObject::connect(slider, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateDenoiseParameter, this, std::placeholders::_1));

    // Threshold label
    //mThresholdLabel = new QLabel;
    //mThresholdLabel->setText("Threshold: 100 HU");
    //menuLayout->addWidget(mThresholdLabel);
    nlmSigmaLabel = new QLabel;
    std::string text2 = "Sigma: " + std::to_string(0.65) + " ";
    nlmSigmaLabel->setText(text2.c_str());
    menuLayout->addWidget(nlmSigmaLabel);
    // Sigma slider
    QSlider* slider2 = new QSlider(Qt::Horizontal);
    slider2->setMinimum(10);
    slider2->setMaximum(3000);
    slider2->setValue(650);
    slider2->setFixedWidth(300);
    menuLayout->addWidget(slider2);

    // Connect the value changed signal of the slider to the updateSigma method
    QObject::connect(slider2, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateSigma, this, std::placeholders::_1));

    //GroupSize Label + Slider
    nlmGroupSizeLabel = new QLabel;
    std::string text3 = "Group Size: " + std::to_string(3) + " ";
    nlmGroupSizeLabel->setText(text3.c_str());
    menuLayout->addWidget(nlmGroupSizeLabel);
    QSlider* slider3 = new QSlider(Qt::Horizontal);
    slider3->setMinimum(3);
    slider3->setMaximum(30);
    slider3->setValue(3);
    slider3->setFixedWidth(300);
    menuLayout->addWidget(slider3);
    QObject::connect(slider3, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateGroupSize, this, std::placeholders::_1));
    
    //WindowSize Label + Slider
    nlmWindowSizeLabel = new QLabel;
    std::string text4 = "Window Size: " + std::to_string(5) + " ";
    nlmWindowSizeLabel->setText(text4.c_str());
    menuLayout->addWidget(nlmWindowSizeLabel);
    QSlider* slider4 = new QSlider(Qt::Horizontal);
    slider4->setMinimum(5);
    slider4->setMaximum(100);
    slider4->setValue(5);
    slider4->setFixedWidth(300);
    menuLayout->addWidget(slider4);
    QObject::connect(slider4, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateWindowSize, this, std::placeholders::_1));
    
    //K Label + Slider
    nlmKLabel = new QLabel;
    std::string text5 = "K Version: " + std::to_string(0) + " ";
    nlmKLabel->setText(text5.c_str());
    menuLayout->addWidget(nlmKLabel);
    QSlider* slider5 = new QSlider(Qt::Horizontal);
    slider5->setMinimum(0);
    slider5->setMaximum(5);
    slider5->setValue(0);
    slider5->setFixedWidth(300);
    menuLayout->addWidget(slider5);
    QObject::connect(slider5, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateK, this, std::placeholders::_1));
    
    //Threshhold
    mThresholdLabel = new QLabel;
    std::string text7 = "Threshold: " + std::to_string(0) + " ";
    mThresholdLabel->setText(text7.c_str());
    menuLayout->addWidget(mThresholdLabel);
    QSlider* slider6 = new QSlider(Qt::Horizontal);
    slider6->setMinimum(1);
    slider6->setMaximum(500);
    slider6->setValue(100);
    slider6->setFixedWidth(300);
    menuLayout->addWidget(slider6);
    QObject::connect(slider6, &QSlider::valueChanged, std::bind(&NLMGUI3D::updateThreshold, this, std::placeholders::_1));
    
    timerLabel = new QLabel;
    std::string text6 = "Kernal time: -- ms" ;
    timerLabel->setText(text6.c_str());
    menuLayout->addWidget(timerLabel);
    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    //layout->addWidget(viewOrig);
    layout->addWidget(view);
    

    mWidget->setLayout(layout);
}
    
void NLMGUI3D::updateThreshold(int value) {
	mSurfaceExtraction->setThreshold(value);

	// Update label
	std::string text = "Threshold: " + std::to_string(value) + " HU";
	mThresholdLabel->setText(text.c_str());
}

void NLMGUI3D::updateDenoiseParameter(int value){
    float newDs = value/1000.0f;
    nlmSmoothing->setDenoiseStrength(newDs);
    //Her skal jeg update label
    std::string text = "Denoise Strength: " + std::to_string(newDs) + " ";
    nlmStrengthLabel->setText(text.c_str());
    nlmSmoothing->getRuntime()->print();
}
void NLMGUI3D::updateGroupSize(int value){
    if(value > 0 && value % 2 == 1){
        nlmSmoothing->setGroupSize((uint)value);
        //update label
        std::string text = "GroupSize: " + std::to_string(value) + " ";
        nlmGroupSizeLabel->setText(text.c_str());
        //nlmSmoothing->update();
        nlmSmoothing->getRuntime()->print();
    }
    
}
void NLMGUI3D::updateWindowSize(int value){
    if(value > 0 && value % 2 ==  1){
        nlmSmoothing->setWindowSize((uint)value);
        //update label
        std::string text = "WindowSize: " + std::to_string(value) + " ";
        nlmWindowSizeLabel->setText(text.c_str());
        nlmSmoothing->getRuntime()->print();
    }
    
}
    
void NLMGUI3D::updateK(int value){
    nlmSmoothing->setK((uint)value);
    std::string text = "Kversion: " + std::to_string(value) + " ";
    nlmKLabel->setText(text.c_str());
    nlmSmoothing->getRuntime()->print();
}
void NLMGUI3D::updateSigma(int value){
    float newS = value/1000.0f;
    nlmSmoothing->setSigma(newS);
    
    
    //update label
    std::string text = "Sigma: " + std::to_string(newS) + " ";
    nlmSigmaLabel->setText(text.c_str());
    nlmSmoothing->getRuntime()->print();
}
}
