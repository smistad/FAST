#include "DummyIGTLImageServer.hpp"
#include "igtlServerSocket.h"
#include "igtlImageMessage.h"
#include "igtlOSUtil.h"
#include "DummyObjects.hpp"

namespace fast {

DummyIGTLImageServer::DummyIGTLImageServer() {
    mFPS = 10;
    mPort = 18944;
}

void DummyIGTLImageServer::setPort(uint port) {
    mPort = port;
}

void DummyIGTLImageServer::setFramesPerSecond(uint fps) {
    mFPS = fps;
}

void DummyIGTLImageServer::setImageStreamer(
        ImageFileStreamer::pointer streamer) {
    mStreamer = streamer;
}

void DummyIGTLImageServer::start() {
    // Create a new thread which runs the server
    mThread = boost::thread(boost::bind(&DummyIGTLImageServer::streamImages, this));
}

DummyIGTLImageServer::~DummyIGTLImageServer() {
    mThread.join();
}

igtl::ImageMessage::Pointer createIGTLImageMessage(Image::pointer image) {
    // size parameters
    int   size[3]     = {(int)image->getWidth(), (int)image->getHeight(), (int)image->getDepth()};       // image dimension
    float spacing[3]  = {image->getSpacing().x(), image->getSpacing().y(), image->getSpacing().z()};     // spacing (mm/pixel)
    int   svoffset[3] = {0, 0, 0};           // sub-volume offset
    int   scalarType;
    size_t totalSize = image->getWidth()*image->getHeight()*image->getDepth()*image->getNrOfComponents();
    switch(image->getDataType()) {
        case TYPE_UINT8:
            scalarType = igtl::ImageMessage::TYPE_UINT8;
            totalSize *= sizeof(unsigned char);
            break;
        case TYPE_INT8:
            scalarType = igtl::ImageMessage::TYPE_INT8;
            totalSize *= sizeof(char);
            break;
        case TYPE_UINT16:
            scalarType = igtl::ImageMessage::TYPE_UINT16;
            totalSize *= sizeof(unsigned short);
            break;
        case TYPE_INT16:
            scalarType = igtl::ImageMessage::TYPE_INT16;
            totalSize *= sizeof(short);
            break;
        case TYPE_FLOAT:
            scalarType = igtl::ImageMessage::TYPE_FLOAT32;
            totalSize *= sizeof(float);
            break;
    }

    igtl::Matrix4x4 matrix;
    igtl::IdentityMatrix(matrix);
    AffineTransformation T = image->getSceneGraphNode()->getAffineTransformation();
    for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
        matrix[i][j] = T.linear()(i,j);
    }}

    //------------------------------------------------------------
    // Create a new IMAGE type message
    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();
    imgMsg->SetDimensions(size);
    imgMsg->SetSpacing(spacing);
    imgMsg->SetOrigin(T.translation().x(), T.translation().y(), T.translation().z());
    imgMsg->SetMatrix(matrix);
    imgMsg->SetNumComponents(image->getNrOfComponents());
    imgMsg->SetScalarType(scalarType);
    imgMsg->SetDeviceName("DummyIGTLImageServer");
    imgMsg->SetSubVolume(size, svoffset);
    imgMsg->AllocateScalars();

    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
    memcpy(imgMsg->GetScalarPointer(), access->get(), totalSize);

    return imgMsg;
}

void DummyIGTLImageServer::streamImages() {
    // Prepare server socket
    igtl::ServerSocket::Pointer serverSocket;
    serverSocket = igtl::ServerSocket::New();
    int r = serverSocket->CreateServer(mPort);
    int interval = (int) (1000.0 / mFPS);

    if(r < 0) {
        throw Exception("Cannot create a server socket.");
    }

    igtl::Socket::Pointer socket;
    DynamicData::pointer dataStream = mStreamer->getOutputData<Image>();
    DummyProcessObject::pointer dummy = DummyProcessObject::New();
    while(true) {
        // Waiting for Connection
        socket = serverSocket->WaitForConnection(1000);
        if(socket.IsNotNull()) { // if client connected
            mStreamer->update();
            while(!dataStream->hasReachedEnd()) {
                // Get next image from streamer
                Image::pointer image = dataStream->getNextFrame(dummy);

                // Create a new IMAGE type message
                igtl::ImageMessage::Pointer imgMsg = createIGTLImageMessage(image);

                imgMsg->Pack();
                socket->Send(imgMsg->GetPackPointer(), imgMsg->GetPackSize());
                igtl::Sleep(interval); // wait
            }
            break;
        }
    }

    socket->CloseSocket();
}


} // end namespace fast
