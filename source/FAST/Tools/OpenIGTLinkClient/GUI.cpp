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

    QObject::connect(recordButton, &QPushButton::clicked, std::bind(&GUI::record, this));

    // Add menu and view to main layout
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(view);

    mWidget->setLayout(layout);
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

        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->addInputConnection(mClient->getOutputPort());
        mStreamer->update();

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
    bool recording = mClient->toggleRecord(storageDir->text().toStdString());
    if(recording) {
        // Start
        recordButton->setText("Stop record");
        recordButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
        storageDir->setDisabled(true);
    } else {
        // Stop
        recordButton->setText("Record");
        recordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        storageDir->setDisabled(false);
    }
}

}
