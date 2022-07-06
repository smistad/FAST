#include "GUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <FAST/PipelineEditor.hpp>
#include <fstream>

namespace fast {

void GUI::updateAvailablePipelines() {
	const auto path = Config::getPipelinePath();
	m_pipelineSelector->clear();
	for(auto&& item : getDirectoryList(path)) {
		try {
			auto pipeline = Pipeline(path + item);
			m_pipelineSelector->addItem(pipeline.getName().c_str(), QString(pipeline.getFilename().c_str()));
		} catch(Exception &e) {
			//reportWarning() << "Error reading pipeline " << item << ": " << e.what() << reportEnd();
		}
	}

}

GUI::GUI() {
	setTitle("FAST - Pipeline");
	m_mainLayout = new QVBoxLayout;
	mWidget->setLayout(m_mainLayout);

	auto menuLayout = new QHBoxLayout;
	m_mainLayout->addLayout(menuLayout);

	m_viewLayout = new QHBoxLayout;
	m_mainLayout->addLayout(m_viewLayout);
	// Add an empty dummy view
	auto view = createView();
	view->setBackgroundColor(Color::Black());
	view->set2DMode();
	m_viewLayout->addWidget(view);

	auto label = new QLabel;
	label->setText("Pipeline:");
	menuLayout->addWidget(label);

	m_pipelineSelector = new QComboBox;
	const std::string pipelinePath = Config::getPipelinePath();
	updateAvailablePipelines();
	menuLayout->addWidget(m_pipelineSelector);

	auto runButton = new QPushButton;
	runButton->setText("Run");
	connect(runButton, &QPushButton::clicked, [this]() {
		m_currentPipeline = m_pipelineSelector->currentData().toInt();
		loadPipeline();
	});
	menuLayout->addWidget(runButton);

	auto stopButton = new QPushButton;
	stopButton->setText("Stop");
	connect(stopButton, &QPushButton::clicked, [this]() {
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
        delete m_viewLayout;
        m_viewLayout = new QHBoxLayout;
        // Add an empty dummy view
        auto view = createView();
        view->setBackgroundColor(Color::Black());
        view->set2DMode();
        m_viewLayout->addWidget(view);
        m_mainLayout->insertLayout(1, m_viewLayout);
	});
	menuLayout->addWidget(stopButton);

	auto editButton = new QPushButton;
	editButton->setText("Edit");
	connect(editButton, &QPushButton::clicked, [this]() {
		std::string filename = m_pipelineSelector->currentData().toString().toStdString();
		auto editor = new PipelineEditor(filename);
		editor->show();
		// TODO stop?
		QObject::connect(editor, &PipelineEditor::saved, [this, filename]() {
			updateAvailablePipelines();
			int index = m_pipelineSelector->findData(QString(filename.c_str()));
			if(index < 0)
				throw Exception("This should not have happened: Could not find the new pipeline..");
			m_pipelineSelector->setCurrentIndex(index);
			loadPipeline();
		});

	});
	menuLayout->addWidget(editButton);

	auto newButton = new QPushButton;
	newButton->setText("New");
	connect(newButton, &QPushButton::clicked, [this, pipelinePath, editButton]() {
		// Ask user for pipeline filename
		QString filename;
		bool ok;
		do {
			filename = QInputDialog::getText(mWidget, "New pipeline",
				"Pipeline filename:", QLineEdit::Normal,
				"", &ok);
			if(!ok || filename.isEmpty()) {
				ok = false;
				break;
			}
			if(fileExists(pipelinePath + filename.toStdString() + ".fpl")) { // We don't want to overwrite an existing file
				int ret = QMessageBox::critical(mWidget, "Error", "A pipeline with that name already exists");
			} else {
				break;
			}
		} while(true);

		if(ok) {
			// Ask user for pipeline name
			QString name = QInputDialog::getText(mWidget, "New pipeline",
				"Pipeline name:", QLineEdit::Normal,
				"", &ok);
			if(!ok)
				return;
			// Create a file, refresh pipelines, select pipeline and issue editPipeline
			std::ofstream file(pipelinePath + filename.toStdString() + ".fpl");
			file << "PipelineName \"" + name.toStdString() + "\"\n";
			file << "PipelineDescription \"\"\n";
			file.close();

			updateAvailablePipelines();

			int index = m_pipelineSelector->findData(QString(pipelinePath.c_str()) + filename + ".fpl");
			if(index < 0)
				throw Exception("This should not have happened: Could not find the new pipeline..");
			m_pipelineSelector->setCurrentIndex(index);
			emit editButton->clicked();
		}
	});
	menuLayout->addWidget(newButton);

	// Add some stretch to keep GUI minialized
	menuLayout->addStretch();
}

void GUI::loadPipeline() {
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
		std::string filename = m_pipelineSelector->currentData().toString().toStdString();
		Pipeline pipeline(filename, m_variables);
		pipeline.parse();

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
		m_mainLayout->insertLayout(1, m_viewLayout);

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
	} catch(Exception &e) {
		std::string errorMessage = e.what();
		int ret = QMessageBox::critical(mWidget, "Pipeline",
                               ("An error occured while opening the pipeline file: " + errorMessage).c_str(),
                               QMessageBox::Ok,
                               QMessageBox::Ok);
		return;
	}

}

void GUI::setPipelineFile(std::string file, std::map<std::string, std::string> variables) {
    // Need this to make sure View is initialized properly..
    int screenHeight = getScreenHeight();
    int screenWidth = getScreenWidth();
    reportInfo() << "Resizing window to " << mWidth << " " << mHeight << reportEnd();
    mWidget->resize(mWidth, mHeight);
    // Move window to center
    int x = (screenWidth - mWidth) / 2;
    int y = (screenHeight - mHeight) / 2;
    mWidget->move(x, y);
    mWidget->show();

	m_variables = variables;
	Pipeline pipeline(file, m_variables);
	auto filename = QString(pipeline.getFilename().c_str());
	m_pipelineSelector->addItem(pipeline.getName().c_str(), filename);
	int index = m_pipelineSelector->findData(filename);
	if(index < 0)
		throw Exception("This should not have happened: Could not find the new pipeline..");
	m_pipelineSelector->setCurrentIndex(index);
	loadPipeline();
}

}