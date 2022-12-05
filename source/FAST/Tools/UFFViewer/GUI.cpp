#include "GUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QSlider>
#include <QTimer>
#include <FAST/Streamers/UFFStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Pipeline.cpp>
#include <FAST/PipelineEditor.hpp>
#include <FAST/Utility.hpp>

namespace fast {

UFFViewerWindow::UFFViewerWindow() {
	setTitle("Ultrasound File Format FAST Viewer");
	enableMaximized();

	// Create layouts
	auto mainLayout = new QHBoxLayout;
	mWidget->setLayout(mainLayout);

	auto menuLayout = new QVBoxLayout;
	menuLayout->setAlignment(Qt::AlignTop);
	mainLayout->addLayout(menuLayout);

	m_rightSideLayout = new QVBoxLayout;
	mainLayout->addLayout(m_rightSideLayout);

	m_viewLayout = new QHBoxLayout;
	m_rightSideLayout->addLayout(m_viewLayout);

	// Add an empty dummy view
	auto view = createView();
	view->setBackgroundColor(Color::Black());
	view->set2DMode();
	m_viewLayout->addWidget(view);

	// Logo
	auto logo = new QLabel;
	logo->setText("Ultrasound File Format Viewer");
	QFont font;
	font.setBold(true);
	//font.setPointSize(14);
	logo->setFont(font);
	menuLayout->addWidget(logo);

	// File selection
	auto selectFileButton = new QPushButton;
	selectFileButton->setText("Open UFF file");
	menuLayout->addWidget(selectFileButton);

	auto fileLabel = new QLabel;
	menuLayout->addWidget(fileLabel);

	QObject::connect(selectFileButton, &QPushButton::clicked, [this, fileLabel]() {
		auto filename = QFileDialog::getOpenFileName(mWidget, "Open File", NULL,
			"Ultrasound File Format (UFF) (*.uff *.hd5 *.hdf5)");
		if(!filename.isEmpty()) {
			m_filename = filename.toStdString();
			//fileLabel->setText(m_filename.substr(m_filename.rfind("/")+1).c_str());
			loadView();
		}
	});

	// Pipeline selection and edit
	auto pipelineLabel = new QLabel;
	pipelineLabel->setText("Pipeline:");
	menuLayout->addWidget(pipelineLabel);

	auto pipelineSelection = new QComboBox;
	menuLayout->addWidget(pipelineSelection);
	const std::string path = Config::getPipelinePath() + "/uff_viewer/";
	std::vector<std::string> pipelineFiles;
	for(auto&& item : getDirectoryList(Config::getPipelinePath() + "/uff_viewer/")) {
		try {
			auto pipeline = Pipeline(path + item);
			pipelineSelection->addItem(pipeline.getName().c_str());
			pipelineFiles.push_back(item);
		} catch(Exception &e) {
			reportWarning() << "Error reading pipeline " << item << ": " << e.what() << reportEnd();
		}
	}
	pipelineSelection->setCurrentText(QString("Default"));

	auto editButton = new QPushButton;
	menuLayout->addWidget(editButton);
	editButton->setText("Edit");
	QObject::connect(editButton, &QPushButton::clicked, [this, path, pipelineSelection, pipelineFiles]() {
		auto selectedFile = pipelineFiles[pipelineSelection->currentIndex()];
		auto editor = new PipelineEditor(path + selectedFile);
		editor->show();
		if(m_streamer)
			m_streamer->setPause(true);
		QObject::connect(editor, &PipelineEditor::saved, [this, selectedFile]() {
			m_pipelineFile = selectedFile;
			loadView();
		});
	});

	auto runButton = new QPushButton;
	menuLayout->addWidget(runButton);
	runButton->setText("Run");
	QObject::connect(runButton, &QPushButton::clicked, [this, pipelineSelection, pipelineFiles]() {
		m_pipelineFile = pipelineFiles[pipelineSelection->currentIndex()];
		loadView();
	});

	// Framerate control
	auto framerateLabel = new QLabel;
	framerateLabel->setText("Framerate:");
	menuLayout->addWidget(framerateLabel);

	m_framerateInput = new QComboBox;
	for(int i = 1; i < 80; ++i)
		m_framerateInput->addItem(QString(std::to_string(i).c_str()));
	m_framerateInput->setCurrentIndex(m_framerate - 1);
	menuLayout->addWidget(m_framerateInput);
	QObject::connect(m_framerateInput, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		m_framerate = index + 1;
		if(m_streamer)
			m_streamer->setFramerate(m_framerate);
	});

    auto gainLabel = new QLabel;
    gainLabel->setText("Gain:");
    menuLayout->addWidget(gainLabel);

    m_gainInput = new QComboBox;
    for(int i = 1; i < 100; ++i)
        m_gainInput->addItem(QString(std::to_string(i).c_str()));
    m_gainInput->setCurrentIndex(9);
    menuLayout->addWidget(m_gainInput);
    QObject::connect(m_gainInput, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_streamer->setGain(index+1);
	});

    auto dynamicRangeLabel = new QLabel;
    dynamicRangeLabel->setText("Dynamic range:");
    menuLayout->addWidget(dynamicRangeLabel);

    m_dynamicRangeInput = new QComboBox;
    for(int i = 1; i < 100; ++i)
        m_dynamicRangeInput->addItem(QString(std::to_string(i).c_str()));
    m_dynamicRangeInput->setCurrentIndex(59);
    menuLayout->addWidget(m_dynamicRangeInput);
    QObject::connect(m_dynamicRangeInput, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_streamer->setDynamicRange(index+1);
    });

	// Playback controls
	auto playbackLayout = new QHBoxLayout;
	m_rightSideLayout->addLayout(playbackLayout);

	m_playButton = new QPushButton;
	m_playButton->setText("Play");
	playbackLayout->addWidget(m_playButton);
	QObject::connect(m_playButton, &QPushButton::clicked, [this]() {
		if(m_streamer) {
			if(!m_streamer->getPause()) {
				m_streamer->setCurrentFrameIndex(m_slider->sliderPosition()); // This will pause as well
			} else {
				m_streamer->setPause(false);
			}
		}
	});

	m_slider = new QSlider;
	m_slider->setOrientation(Qt::Horizontal);
	playbackLayout->addWidget(m_slider);
	QObject::connect(m_slider, &QSlider::sliderMoved, [this](int index) {
		if(m_streamer) {
			m_streamer->setCurrentFrameIndex(index);
		}
	});

	// Playback slider update
	auto timer = new QTimer;
	timer->setInterval(10);
	timer->setSingleShot(false);
	QObject::connect(timer, &QTimer::timeout, [this]() {
		if(m_streamer) {
			m_slider->setSliderPosition(m_streamer->getCurrentFrameIndex()); // This is not entirely correct as the streamer may have sent out more frames than has been visualized.. can be fixed by setting maximumNrOfFrames to 1
			if(m_streamer->getPause()) {
				m_playButton->setText("Play");
			} else {
				m_playButton->setText("Pause");
			}
		}

	});
	timer->start();
}

void UFFViewerWindow::setFilename(std::string filename) {
	m_filename = filename;
	loadView();
}

void UFFViewerWindow::loadView() {
    // Set up pipeline and view
    try {
        m_streamer = UFFStreamer::create(
                m_filename,
                true,
                m_framerate,
                m_gainInput->currentIndex()+1,
                m_dynamicRangeInput->currentIndex()+1
        );
        m_streamer->setMaximumNrOfFrames(1); // To avoid glitches in playback we set the queue size to 1
    } catch(Exception &e) {
        std::string errorMessage = e.what();
        int ret = QMessageBox::critical(mWidget, "UFF Viewer",
                                        ("An error occured while opening the file: " + errorMessage).c_str(),
                                        QMessageBox::Ok,
                                        QMessageBox::Ok);
        return;
    }

    // Set up pipeline and view
    try {
        auto thread = getComputationThread();
        thread->clearViews();
        thread->clearProcessObjects();

        // Remove and delete old views
        auto views = getViews();
        clearViews();
        for(auto view : views) {
            view->stopPipeline();
            delete view;
        }

        // Load pipeline (must be done after stopComputationThread)
        Pipeline pipeline(Config::getPipelinePath() + "/uff_viewer/" + m_pipelineFile);
        pipeline.parse({}, {{"UFFstream", m_streamer}});

        // Setup renderers and views
        auto renderers = pipeline.getRenderers();

        // Disable renderers for now
        for(auto&& renderer : renderers)
            renderer->setDisabled(true);

        // Add all pipeline views to the window:
        for(auto view : pipeline.getViews()) {
            view->setAutoUpdateCamera(true);
            addView(view);
        }

        // Recreate the view layout
        delete m_viewLayout;
        m_viewLayout = new QHBoxLayout;
        for(auto view : pipeline.getViews()) {
            m_viewLayout->addWidget(view);
        }
        m_rightSideLayout->insertLayout(0, m_viewLayout);

        // Enable renderers again
        for(auto&& renderer : renderers) {
            renderer->update();
            renderer->setDisabled(false);
        }

        for(auto&& po : pipeline.getProcessObjects()) {
            if(po.second->getNrOfOutputPorts() == 0 && std::dynamic_pointer_cast<Renderer>(po.second) == nullptr) {
                // Process object has no output ports, must add to window to make sure it is updated.
                reportInfo() << "Process object " << po.first << " had no output ports defined in pipeline, therefore adding to window so it is updated." << reportEnd();
                addProcessObject(po.second);
            }
        }

        for(auto view : pipeline.getViews()) {
            view->reinitialize();
        }
        m_slider->setRange(0, m_streamer->getNrOfFrames()-1); // This must be called after update, due to refactoring in UFFStreamer
    } catch(Exception &e) {
        std::string errorMessage = e.what();
        int ret = QMessageBox::critical(mWidget, "Pipeline",
                                        ("An error occured while opening the pipeline file: " + errorMessage).c_str(),
                                        QMessageBox::Ok,
                                        QMessageBox::Ok);
        return;
    }
}

}