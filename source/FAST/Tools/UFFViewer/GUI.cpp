#include "GUI.hpp""
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <FAST/Streamers/UFFStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

namespace fast {

UFFViewerWindow::UFFViewerWindow() {
	Reporter::setGlobalReportMethod(Reporter::COUT);
	setTitle("Ultrasound File Format FAST Viewer");
	auto mainLayout = new QHBoxLayout;
	mWidget->setLayout(mainLayout);

	auto menuLayout = new QVBoxLayout;
	menuLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(menuLayout);

	m_viewLayout = new QHBoxLayout;
	mainLayout->addLayout(m_viewLayout);
	auto view = createView();
	view->setBackgroundColor(Color::Black());
	view->set2DMode();
	//view->setAutoUpdateCamera(true);
	m_viewLayout->addWidget(view);

	auto logo = new QLabel;
	logo->setText("Ultrasound File Format Viewer");
	QFont font;
	font.setBold(true);
	//font.setPointSize(14);
	logo->setFont(font);
	menuLayout->addWidget(logo);

	auto selectFileButton = new QPushButton;
	selectFileButton->setText("Open UFF file");
	menuLayout->addWidget(selectFileButton);
	QObject::connect(selectFileButton, &QPushButton::clicked, this, &UFFViewerWindow::selectFile);


	auto framerateLabel = new QLabel;
	framerateLabel->setText("Framerate:");
	menuLayout->addWidget(framerateLabel);

	m_framerateInput = new QComboBox;
	for(int i = 1; i < 80; ++i)
		m_framerateInput->addItem(QString(std::to_string(i).c_str()));
	m_framerateInput->setCurrentIndex(m_framerate - 1);
	menuLayout->addWidget(m_framerateInput);
	QObject::connect(m_framerateInput, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UFFViewerWindow::selectFramerate);
}

void UFFViewerWindow::selectFile() {
	auto filename = QFileDialog::getOpenFileName(mWidget, "Open File", NULL,
                                                "Ultrasound File Format (UFF) (*.uff *.hd5 *.hdf5)");
	if(!filename.isEmpty()) {
		setFilename(filename.toStdString());
		loadView();
	}
}

void UFFViewerWindow::selectFramerate(int index) {
	m_framerate = index + 1;
	if(m_streamer)
		m_streamer->setFramerate(m_framerate);
}

void UFFViewerWindow::setFilename(std::string filename) {
	std::cout << filename << std::endl;
	m_filename = filename;
	loadView();
}

void UFFViewerWindow::loadView() {
	// Set up pipeline and view
	m_streamer = UFFStreamer::New();
	m_streamer->setFilename(m_filename);
	m_streamer->setLooping(true);
	m_streamer->setFramerate(m_framerate);
	/*
	try {
		streamer->update();
	} catch(Exception &e) {
		std::string errorMessage = e.what();
		int ret = QMessageBox::critical(mWidget, "UFF Viewer",
                               ("An error occured while opening the file: " + errorMessage).c_str(),
                               QMessageBox::Ok,
                               QMessageBox::Ok);
		return;
	}*/

	auto imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(m_streamer->getOutputPort());
	imageRenderer->setSynchronizedRendering(false);

	getView(0)->removeAllRenderers();
	getView(0)->addRenderer(imageRenderer);
	getView(0)->reinitialize();

}

}