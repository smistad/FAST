#include "DummyIGTLServer.hpp"

#include <igtl/igtlServerSocket.h>
#include <igtl/igtlImageMessage.h>
#include <igtl/igtlTransformMessage.h>
#include <igtl/igtlOSUtil.h>
#include "FAST/Tests/DummyObjects.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

DummyIGTLServer::DummyIGTLServer() {
    mFPS = 10;
    mPort = 18944;
    mFrames = std::numeric_limits<int>::max();
}

void DummyIGTLServer::setPort(uint port) {
    mPort = port;
}

void DummyIGTLServer::setFramesPerSecond(uint fps) {
    mFPS = fps;
}

void DummyIGTLServer::setMaximumFramesToSend(uint frames) {
    mFrames = frames;
}

void DummyIGTLServer::setImageStreamer(
        ImageFileStreamer::pointer streamer) {
    mStreamer = streamer;
}

void DummyIGTLServer::start() {
    // Create a new thread which runs the server
    mThread = std::thread(std::bind(&DummyIGTLServer::stream, this));
}

DummyIGTLServer::~DummyIGTLServer() {
    mThread.join();
}

inline igtl::ImageMessage::Pointer createIGTLImageMessage(Image::pointer image) {
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

    //------------------------------------------------------------
    // Create a new IMAGE type message
    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();
    imgMsg->SetDimensions(size);
    imgMsg->SetSpacing(spacing);
    imgMsg->SetNumComponents(image->getNrOfComponents());
    imgMsg->SetScalarType(scalarType);
    imgMsg->SetDeviceName("DummyImage");
    imgMsg->SetSubVolume(size, svoffset);
    imgMsg->AllocateScalars();

    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
    memcpy(imgMsg->GetScalarPointer(), access->get(), totalSize);

    return imgMsg;
}

inline igtl::TransformMessage::Pointer createIGTLTransformMessage(Image::pointer image) {
    // Create transform message from the scene graph information of image
    igtl::Matrix4x4 matrix;
    AffineTransformation::pointer T = image->getSceneGraphNode()->getTransformation();
    for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
        matrix[i][j] = T->getTransform().matrix()(i,j);
    }}

    igtl::TransformMessage::Pointer message = igtl::TransformMessage::New();
    message->SetDeviceName("DummyTransform");
    message->SetMatrix(matrix);

    return message;
}

void DummyIGTLServer::stream() {
    // Prepare server socket
    igtl::ServerSocket::Pointer serverSocket = igtl::ServerSocket::New();
    int r = serverSocket->CreateServer(mPort);

    if(r < 0) {
        throw Exception("Cannot create a server socket.");
    }

    igtl::Socket::Pointer socket;
    DynamicData::pointer dataStream = mStreamer->getOutputData<Image>();
    DummyProcessObject::pointer dummy = DummyProcessObject::New();
    int framesSent = 0;
    int interval = (int) (1000.0 / mFPS);
    while(true) {
        // Waiting for Connection
        socket = serverSocket->WaitForConnection(1000);
        if(socket.IsNotNull()) { // if client connected
            mStreamer->update();
            while(!dataStream->hasReachedEnd() && framesSent < mFrames) {
                // Get next image from streamer
                Image::pointer image = dataStream->getNextFrame(dummy);

                // Create a new IMAGE type message
                igtl::ImageMessage::Pointer imgMsg = createIGTLImageMessage(image);

                imgMsg->Pack();
                socket->Send(imgMsg->GetPackPointer(), imgMsg->GetPackSize());

                // Create a new TRANSFORM type message
                igtl::TransformMessage::Pointer transformMsg = createIGTLTransformMessage(image);

                transformMsg->Pack();
                socket->Send(transformMsg->GetPackPointer(), transformMsg->GetPackSize());

                igtl::Sleep(interval); // wait
                framesSent++;
            }
            break;
        }
    }

    socket->CloseSocket();
    serverSocket->CloseSocket();
    Reporter::info() << "Closed IGT Link server socket" << Reporter::end();
}


} // end namespace fast
