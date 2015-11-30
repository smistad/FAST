/**
 * Examples/GUI/SimpleGUI/SimpleGUI.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "SimpleGUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/MeshRenderer/MeshRenderer.hpp>
#include <boost/bind.hpp>

namespace fast {

SimpleGUI::SimpleGUI() {

	// Create a 3D view
    View* view = createView();
    view->set3DMode();

    enableFullscreen();

    // Import image
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "/CT-Abdomen.mhd");

    // Smooth image
    mSmoothing = GaussianSmoothingFilter::New();
    mSmoothing->setInputConnection(importer->getOutputPort());
    mSmoothing->setStandardDeviation(1);

    // Set up surface extraction
    mSurfaceExtraction = SurfaceExtraction::New();
    mSurfaceExtraction->setInputConnection(mSmoothing->getOutputPort());
    mSurfaceExtraction->setThreshold(100);

    // Set up rendering
    MeshRenderer::pointer renderer = MeshRenderer::New();
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
    QObject::connect(quitButton, &QPushButton::clicked, boost::bind(&Window::stop, this));

    // Smoothing parameter label
    mSmoothingLabel = new QLabel;
    mSmoothingLabel->setText("Smoothing: 1.5 mm");
    menuLayout->addWidget(mSmoothingLabel);

    // Smoothing parameter slider
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(3);
    slider->setValue(1);
    slider->setFixedWidth(200);
    menuLayout->addWidget(slider);

    // Connect the value changed signal of the slider to the updateSmoothingParameter method
    QObject::connect(slider, &QSlider::valueChanged, boost::bind(&SimpleGUI::updateSmoothingParameter, this, _1));

    // Threshold label
    mThresholdLabel = new QLabel;
    mThresholdLabel->setText("Threshold: 100 HU");
    menuLayout->addWidget(mThresholdLabel);

    // Threshold slider
    QSlider* slider2 = new QSlider(Qt::Horizontal);
    slider2->setMinimum(-100);
    slider2->setMaximum(300);
    slider2->setValue(100);
    slider2->setFixedWidth(200);
    menuLayout->addWidget(slider2);

    // Connect the value changed signal of the slider to the updateThreshold method
    QObject::connect(slider2, &QSlider::valueChanged, boost::bind(&SimpleGUI::updateThreshold, this, _1));

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(view);

    mWidget->setLayout(layout);
}

void SimpleGUI::updateThreshold(int value) {
	mSurfaceExtraction->setThreshold(value);

	// Update label
	std::string text = "Threshold: " + boost::lexical_cast<std::string>(value) + " HU";
	mThresholdLabel->setText(text.c_str());
}

void SimpleGUI::updateSmoothingParameter(int value) {
	// The slider can only use int values, must convert to float manually
	float standardDeviation = 1 + value*0.5;
	mSmoothing->setStandardDeviation(standardDeviation);

	// Update label
	std::string text = "Smoothing: " + boost::lexical_cast<std::string>(standardDeviation) + " mm";
	mSmoothingLabel->setText(text.c_str());
}

}
