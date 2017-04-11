#include "GUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QImage>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Streamers/IGTLinkStreamer.hpp>
#include "OpenIGTLinkClient.hpp"
#include <QMessageBox>


namespace fast {

GUI::GUI() {

    const int menuWidth = 300;

    mClient = OpenIGTLinkClient::New();
    mConnected = false;

    // Create a 2D view
    View* view = createView();
    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    setWidth(1280);
    setHeight(768);
    setTitle("FAST - OpenIGTLink Client");

    // Create and add GUI elements

    // First create the menu layout
    QVBoxLayout* menuLayout = new QVBoxLayout;

    // Menu items should be aligned to the top
    menuLayout->setAlignment(Qt::AlignTop);

    // Logo
    QImage* image = new QImage;
    image->load("/home/smistad/workspace/FAST/FAST_logo_square.png");
    QLabel* logo = new QLabel;
    logo->setPixmap(QPixmap::fromImage(image->scaled(300, (300.0f/image->width())*image->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    logo->adjustSize();
    menuLayout->addWidget(logo);

    // Title label
    QLabel* title = new QLabel;
    title->setText("OpenIGTLink\nClient");
    QFont font;
    font.setPointSize(24);
    title->setFont(font);
    menuLayout->addWidget(title);

    // Quit button
    QPushButton* quitButton = new QPushButton;
    quitButton->setText("Quit");
    quitButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    quitButton->setFixedWidth(menuWidth);
    menuLayout->addWidget(quitButton);

    // Connect the clicked signal of the quit button to the stop method for the window
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));

    // Address and port
    QLabel* addressLabel = new QLabel;
    addressLabel->setText("Server address");
    menuLayout->addWidget(addressLabel);

    address = new QLineEdit;
    address->setText("localhost");
    address->setFixedWidth(menuWidth);
    menuLayout->addWidget(address);

    QLabel* portLabel = new QLabel;
    portLabel->setText("Server port");
    menuLayout->addWidget(portLabel);

    port = new QLineEdit;
    port->setText("18944");
    port->setFixedWidth(menuWidth);
    menuLayout->addWidget(port);

    connectButton = new QPushButton;
    connectButton->setText("Connect");
    connectButton->setFixedWidth(menuWidth);
    connectButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    menuLayout->addWidget(connectButton);

    QObject::connect(connectButton, &QPushButton::clicked, std::bind(&GUI::connect, this));

    QLabel* storageDirLabel = new QLabel;
    storageDirLabel->setText("Storage directory");
    menuLayout->addWidget(storageDirLabel);

    storageDir = new QLineEdit;
    storageDir->setText(QDir::homePath() + QDir::separator() + QString("FAST_Recordings"));
    storageDir->setFixedWidth(menuWidth);
    menuLayout->addWidget(storageDir);

    recordButton = new QPushButton;
    recordButton->setText("Record");
    recordButton->setFixedWidth(menuWidth);
    recordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    menuLayout->addWidget(recordButton);

    recordingInformation = new QLabel;
    recordingInformation->setFixedWidth(menuWidth);
    menuLayout->addWidget(recordingInformation);

    QObject::connect(recordButton, &QPushButton::clicked, std::bind(&GUI::record, this));

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(view);

    mWidget->setLayout(layout);

    // Update messages frequently
    QTimer* timer = new QTimer(this);
    timer->start(1000/5); // in milliseconds
    timer->setSingleShot(false);
    QObject::connect(timer, &QTimer::timeout, std::bind(&GUI::updateMessages, this));
}

void GUI::connect() {
    if(mConnected) {
        mStreamer->stop(); // This should probably block until it has stopped
        //getView(0)->removeAllRenderers();

        connectButton->setText("Connect");
        connectButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        address->setDisabled(false);
        port->setDisabled(false);
        mConnected = false;
    } else {
        mStreamer = IGTLinkStreamer::New();
        mStreamer->setConnectionAddress(address->text().toStdString());
        mStreamer->setConnectionPort(std::stoi(port->text().toStdString()));
        mClient->setInputConnection(mStreamer->getOutputPort<Image>("tissue"));
        try {
            mStreamer->update();
        } catch(Exception &e) {
            QMessageBox* message = new QMessageBox;
            message->setWindowTitle("Error");
            message->setText(e.what());
            message->show();
            return;
        }


        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->addInputConnection(mClient->getOutputPort());

        getView(0)->addRenderer(renderer);
        getView(0)->reinitialize();

        connectButton->setText("Disconnect");
        connectButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
        address->setDisabled(true);
        port->setDisabled(true);
        mConnected = true;
    }
}

void GUI::record() {
    if(!mConnected) {
        // Can't record if not connected
        QMessageBox* message = new QMessageBox;
        message->setWindowTitle("Error");
        message->setText("You have to connect to a server before recording.");
        message->show();
        return;
    }
    bool recording = mClient->toggleRecord(storageDir->text().toStdString());
    if(recording) {
        std::string msg = "Recording to: " + mClient->getRecordingName();
        recordingInformation->setText(msg.c_str());
        // Start
        recordButton->setText("Stop recording");
        recordButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
        storageDir->setDisabled(true);
    } else {
        // Stop
        recordingInformation->setText("");
        recordButton->setText("Record");
        recordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        storageDir->setDisabled(false);
    }
}

void GUI::updateMessages() {
    if(mClient->isRecording()) {
        std::string msg = "Recording to: " + mClient->getRecordingName() + "\n";
        msg += std::to_string(mClient->getFramesStored()) + " frames stored";
        recordingInformation->setText(msg.c_str());
    }
}

}
