#include "GUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QImage>
#include <QInputDialog>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/TextRenderer/TextRenderer.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QComboBox>
#include <QDesktopServices>
#include <QListWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <FAST/PipelineEditor.hpp>
#include <fstream>
#include <QDesktopWidget>
#include <QApplication>


namespace fast {

GUI::GUI() {

    QDesktopWidget* desktop = QApplication::desktop();
    menuWidth = desktop->width()*(1.0f/6.0f);
    mPipelineWidget = nullptr;
    mStreamer = ImageFileStreamer::New();

    QVBoxLayout* viewLayout = new QVBoxLayout;


    View* view = createView();
    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    setWidth(desktop->width());
    setHeight(desktop->height());
    enableMaximized();
    setTitle("FAST - Viewer");
    viewLayout->addWidget(view);

    menuLayout = new QVBoxLayout;
    menuLayout->setAlignment(Qt::AlignTop);

    // Logo
    QImage* image = new QImage;
    image->load((Config::getDocumentationPath() + "images/FAST_logo_square.png").c_str());
    QLabel* logo = new QLabel;
    logo->setPixmap(QPixmap::fromImage(image->scaled(menuWidth, ((float)menuWidth/image->width())*image->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    logo->adjustSize();
    menuLayout->addWidget(logo);

    // Title label
    QLabel* title = new QLabel;
    title->setText("Viewer");
	QFont font;
	font.setPixelSize(24 * getScalingFactor());
	font.setWeight(QFont::Bold);
	title->setFont(font);
	title->setAlignment(Qt::AlignCenter);
    menuLayout->addWidget(title);

    // Quit button
    QPushButton* quitButton = new QPushButton;
    quitButton->setText("Quit (q)");
    quitButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    quitButton->setFixedWidth(menuWidth);
    menuLayout->addWidget(quitButton);

    // Connect the clicked signal of the quit button to the stop method for the window
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));

    QLabel* inputListLabel = new QLabel;
    //inputListLabel->setFixedHeight(24);
    inputListLabel->setText("Input data");
    inputListLabel->setStyleSheet("QLabel { font-weight: bold; }");
    menuLayout->addWidget(inputListLabel);

    mList = new QListWidget;
    mList->setFixedWidth(menuWidth);
    mList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mList->setFixedHeight(200);
    mList->setSelectionMode(QAbstractItemView::ExtendedSelection); // Allow multiple items to be selected
    QObject::connect(mList, &QListWidget::itemSelectionChanged, std::bind(&GUI::selectInputData, this));
    menuLayout->addWidget(mList);

    QPushButton* addButton = new QPushButton;
    addButton->setText("Add input data");
    addButton->setFixedWidth(menuWidth);
    QObject::connect(addButton, &QPushButton::clicked, std::bind(&GUI::addInputData, this));
    menuLayout->addWidget(addButton);

    QLabel* selectPipelineLabel = new QLabel;
    selectPipelineLabel->setText("Active pipeline");
    selectPipelineLabel->setStyleSheet("QLabel { font-weight: bold; }");
    //selectPipelineLabel->setFixedHeight(24);
    menuLayout->addWidget(selectPipelineLabel);

    mSelectPipeline = new QComboBox;
    mSelectPipeline->setFixedWidth(menuWidth);
    mPipelines = getAvailablePipelines();
    int index = 0;
    int counter = 0;
    for(auto pipeline : mPipelines) {
        mSelectPipeline->addItem((pipeline.getName() + " (" + pipeline.getDescription() + ")").c_str());
        if(pipeline.getName() == "Image renderer") {
            index = counter;
        }
        ++counter;
    }
    mSelectPipeline->setCurrentIndex(index);

    menuLayout->addWidget(mSelectPipeline);
    QObject::connect(mSelectPipeline, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), std::bind(&GUI::selectPipeline, this));

    QPushButton* refreshPipeline = new QPushButton;
    refreshPipeline->setText("Refresh pipeline");
    refreshPipeline->setStyleSheet("QPushButton { background-color: blue; color: white; }");
    refreshPipeline->setFixedWidth(menuWidth);
    QObject::connect(refreshPipeline, &QPushButton::clicked, std::bind(&GUI::selectPipeline, this));
    menuLayout->addWidget(refreshPipeline);

    QPushButton* editPipeline = new QPushButton;
    editPipeline->setText("Edit pipeline");
    editPipeline->setStyleSheet("QPushButton { background-color: blue; color: white; }");
    editPipeline->setFixedWidth(menuWidth);
    QObject::connect(editPipeline, &QPushButton::clicked, std::bind(&GUI::editPipeline, this));
    menuLayout->addWidget(editPipeline);

    QPushButton* newPipeline = new QPushButton;
    newPipeline->setText("New pipeline");
    newPipeline->setStyleSheet("QPushButton { background-color: blue; color: white; }");
    newPipeline->setFixedWidth(menuWidth);
    QObject::connect(newPipeline, &QPushButton::clicked, std::bind(&GUI::newPipeline, this));
    menuLayout->addWidget(newPipeline);

    // Playback
    QHBoxLayout* playbackLayout = new QHBoxLayout;

    mPlayPauseButton = new QPushButton;
    mPlayPauseButton->setText("Play");
    mPlayPauseButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    //mPlayPauseButton->setFixedHeight(100);
    QObject::connect(mPlayPauseButton, &QPushButton::clicked, std::bind(&GUI::playPause, this));
    playbackLayout->addWidget(mPlayPauseButton);

    QSlider* slider = new QSlider(Qt::Horizontal);
    playbackLayout->addWidget(slider);
    slider->setTickInterval(10);
    slider->setRange(0, 1234);
    slider->setTickPosition(QSlider::TicksAbove);

    viewLayout->addLayout(playbackLayout);

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addLayout(viewLayout);

    mWidget->setLayout(layout);
	std::cout << "Finished viewer setup" << std::endl;

}

void GUI::newPipeline() {
    bool ok;
    QString text = QInputDialog::getText(mWidget, "Create new pipeline", "Enter filename of new pipeline:", QLineEdit::Normal, "", &ok);

    if(ok && !text.isEmpty()) {
        std::string filename = (const char*)text.toUtf8();
        // Make sure file ends with .fpl
        if(filename.substr(filename.size() - 3) != "fpl")
            filename += ".fpl";

        // Create pipeline file with template
        std::ofstream file(Config::getPipelinePath() + filename);
        file << "# New pipeline template\n"
         "PipelineName \"<Pipeline name here>\"\n"
        "PipelineDescription \"<Pipeline description here>\"\n\n"

        "# Pipeline needs at least 1 renderer\n"
        "Renderer renderer ImageRenderer\n"
        "Input 0 PipelineInput\n"
        "Attribute window 255\n"
        "Attribute level 127.5\n";
        file.close();

        PipelineEditor *editor = new PipelineEditor(filename);
        QObject::connect(editor, &PipelineEditor::saved, std::bind(&GUI::selectPipeline, this));
        editor->show();
    }
}

void GUI::editPipeline() {
    int selectedPipeline = mSelectPipeline->currentIndex();
    Pipeline pipeline = mPipelines.at(selectedPipeline);
    PipelineEditor* editor = new PipelineEditor(pipeline.getFilename());
    QObject::connect(editor, &PipelineEditor::saved, std::bind(&GUI::selectPipeline, this));
    editor->show();
}

void GUI::selectPipeline() {
    // Stop computation thread before removing renderers
    stopComputationThread();

    std::vector<std::string> inputData;
    for(QListWidgetItem* widget : mList->selectedItems()) {
		std::string asd = widget->text().toUtf8().constData();
        inputData.push_back(asd);
    }
    mStreamer = ImageFileStreamer::New();
    mStreamer->setFilenameFormats(inputData);
    mStreamer->enableLooping();
    mStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mStreamer->setSleepTime(50);

    getView(0)->removeAllRenderers();

    int selectedPipeline = mSelectPipeline->currentIndex();
    Pipeline pipeline = mPipelines.at(selectedPipeline);
    try {
        std::vector<SharedPointer<Renderer>> renderers = pipeline.setup(mStreamer->getOutputPort());
        for(auto renderer : renderers) {
            // A hack for text renderer which needs a reference to the view
            if(renderer->getNameOfClass() == "TextRenderer") {
                TextRenderer::pointer textRenderer = renderer;
                textRenderer->setView(getView(0));
            }
            getView(0)->addRenderer(renderer);
        }
        getView(0)->reinitialize();
    } catch(Exception &e) {
        QMessageBox* message = new QMessageBox;
        message->setWindowTitle("Error");
        message->setText(e.what());
        message->show();
    }

    startComputationThread();

    /*
    PipelineWidget* pipelineWidget = new PipelineWidget(pipeline, mWidget);
    pipelineWidget->setFixedWidth(menuWidth);
    if(mPipelineWidget == nullptr) {
        menuLayout->addWidget(pipelineWidget);
    } else {
        menuLayout->replaceWidget(mPipelineWidget, pipelineWidget);
    }
    mPipelineWidget = pipelineWidget;
     */
}

void GUI::selectInputData() {
    selectPipeline();
}

void GUI::addInputData() {
    QFileDialog fileDialog(mWidget);
    fileDialog.setNameFilter("Image recordings (*_0.mhd *_0.png *_0.jpg *_0.bmp)");
    QStringList filenames;
    if(fileDialog.exec()) {
        filenames = fileDialog.selectedFiles();
        for(QString qfilename : filenames) {
			std::string filename = qfilename.toUtf8().constData();
            filename = replace(filename, "_0.", "_#.");
            mList->addItem(filename.c_str());
        }
    }
}

void GUI::playPause() {
    if(mPlayPauseButton->text() == "Play") {
        mPlayPauseButton->setText("Pause");
        mPlayPauseButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    } else {
        mPlayPauseButton->setText("Play");
        mPlayPauseButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    }
}

}
