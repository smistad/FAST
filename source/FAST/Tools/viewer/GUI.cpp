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


namespace fast {

GUI::GUI() {

    menuWidth = 300;
    mPipelineWidget = nullptr;
    mStreamer = ImageFileStreamer::New();

    QVBoxLayout* viewLayout = new QVBoxLayout;


    View* view = createView();
    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    setWidth(1280);
    setHeight(768);
    enableMaximized();
    setTitle("FAST - Viewer");
    viewLayout->addWidget(view);

    menuLayout = new QVBoxLayout;

    // Logo
    QImage* image = new QImage;
    image->load((Config::getDocumentationPath() + "images/FAST_logo_square.png").c_str());
    QLabel* logo = new QLabel;
    logo->setPixmap(QPixmap::fromImage(image->scaled(menuWidth, ((float)menuWidth/image->width())*image->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    logo->adjustSize();
    menuLayout->addWidget(logo);

    // Title label
    QLabel* title = new QLabel;
    title->setText("<div style=\"text-align: center; font-weight: bold; font-size: 24px;\">Viewer</div>");
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
    inputListLabel->setText("Input data");
    menuLayout->addWidget(inputListLabel);

    mList = new QListWidget;
    mList->setFixedWidth(menuWidth);
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

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addLayout(viewLayout);

    mWidget->setLayout(layout);

}

/*
void GUI::newPipeline() {
    QInputDialog* dialog = new QInputDialog(this) ;
    dialog->setLabelText("Enter filename of new pipeline");
    dialog->setOkButtonText("Create pipeline");
    dialog->show();
}
 */

void GUI::editPipeline() {
    int selectedPipeline = mSelectPipeline->currentIndex();
    Pipeline pipeline = mPipelines.at(selectedPipeline);
    QDesktopServices::openUrl(QUrl(("file://" + pipeline.getFilename()).c_str()));
}

void GUI::selectPipeline() {
    // Stop computation thread before removing renderers
    stopComputationThread();
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
            std::cout << "added a renderer" << std::endl;
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
    std::cout << "in selectInputData" << std::endl;
    std::vector<std::string> inputData;
    for(QListWidgetItem* widget : mList->selectedItems()) {
        inputData.push_back(widget->text().toStdString());
        std::cout << "selecting " << widget->text().toStdString() << std::endl;
    }
    mStreamer = ImageFileStreamer::New();
    mStreamer->setFilenameFormats(inputData);
    mStreamer->enableLooping();
    mStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mStreamer->setSleepTime(50);
    selectPipeline();
}

void GUI::addInputData() {
    QFileDialog fileDialog(mWidget);
    fileDialog.setNameFilter("Image recordings (*_0.mhd *_0.png *_0.jpg *_0.bmp)");
    QStringList filenames;
    if(fileDialog.exec()) {
        filenames = fileDialog.selectedFiles();
        for(QString qfilename : filenames) {
            std::string filename = qfilename.toStdString();
            filename = replace(filename, "_0.", "_#.");
            mList->addItem(filename.c_str());
        }
    }
}

}
