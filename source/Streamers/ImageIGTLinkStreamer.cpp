#include "ImageIGTLinkStreamer.hpp"
#include "Image.hpp"
#include <boost/shared_array.hpp>

#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlTransformMessage.h"
#include "igtlPositionMessage.h"
#include "igtlImageMessage.h"
#include "igtlStatusMessage.h"

namespace fast {

void ImageIGTLinkStreamer::setConnectionAddress(std::string address) {
    mAddress = address;
    mIsModified = true;
}

void ImageIGTLinkStreamer::setConnectionPort(uint port) {
    mPort = port;
    mIsModified = true;
}

void ImageIGTLinkStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);}

void ImageIGTLinkStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    DynamicData::pointer data = getOutputData<DynamicData >();
    data->setMaximumNumberOfFrames(nrOfFrames);
}

bool ImageIGTLinkStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

uint ImageIGTLinkStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

Image::pointer createFASTImageFromMessage(igtl::ImageMessage::Pointer message, ExecutionDevice::pointer device) {
    Image::pointer image = Image::New();
    int width, height, depth;
    message->GetDimensions(width, height, depth);
    void* data = message->GetScalarPointer();
    DataType type;
    switch(message->GetScalarType()) {
        case igtl::ImageMessage::TYPE_INT8:
            type = TYPE_INT8;
            break;
        case igtl::ImageMessage::TYPE_UINT8:
            type = TYPE_UINT8;
            break;
        case igtl::ImageMessage::TYPE_INT16:
            type = TYPE_INT16;
            break;
        case igtl::ImageMessage::TYPE_UINT16:
            type = TYPE_UINT16;
            break;
        case igtl::ImageMessage::TYPE_FLOAT32:
            type = TYPE_FLOAT;
            break;
        default:
            throw Exception("Unsupported image data type.");
            break;
    }

    if(depth == 1) {
        image->create2DImage(width, height, type, message->GetNumComponents(), device, data);
    } else {
        image->create3DImage(width, height, depth, type, message->GetNumComponents(), device, data);
    }

    boost::shared_array<float> spacing(new float[3]);
    boost::shared_array<float> offset(new float[3]);
    message->GetSpacing(spacing.get());
    message->GetOrigin(offset.get());
    igtl::Matrix4x4 matrix;
    message->GetMatrix(matrix);
    image->setSpacing(Vector3f(spacing[0], spacing[1], spacing[2]));
    image->setOffset(Vector3f(offset[0], offset[1], offset[2]));
    Matrix3f fastMatrix;
    for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
        fastMatrix(i,j) = matrix[i][j];
    }}
    image->setTransformMatrix(fastMatrix);
    igtl::PrintMatrix(matrix);
    std::cout << fastMatrix << std::endl;
    std::cout << "SPACING IS " << spacing[0] << std::endl;

    // TODO transform matrix

    return image;
}

void ImageIGTLinkStreamer::producerStream() {
    mSocket = igtl::ClientSocket::New();
    int r = mSocket->ConnectToServer(mAddress.c_str(), mPort);
    if(r != 0) {
       throw Exception("Cannot connect to the Open IGT Link server.");
    }

    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();

    // Allocate a time stamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();

    while(true) {

        // Initialize receive buffer
        headerMsg->InitPack();

        // Receive generic header from the socket
        int r = mSocket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
        std::cout << "Recieved header.." << std::endl;
        if(r == 0) {
           mSocket->CloseSocket();
           break;
        }
        if(r != headerMsg->GetPackSize()) {
           continue;
        }

        // Deserialize the header
        headerMsg->Unpack();

        // Get time stamp
        igtlUint32 sec;
        igtlUint32 nanosec;
        headerMsg->GetTimeStamp(ts);
        ts->GetTimeStamp(&sec, &nanosec);

        std::cout << "Time stamp: "
           << sec << "." << std::setw(9) << std::setfill('0')
           << nanosec << std::endl;
        if(strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0) {
            std::cout << "Receiving IMAGE data type." << std::endl;

            // Create a message buffer to receive transform data
            igtl::ImageMessage::Pointer imgMsg;
            imgMsg = igtl::ImageMessage::New();
            imgMsg->SetMessageHeader(headerMsg);
            imgMsg->AllocatePack();

            // Receive transform data from the socket
            mSocket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

            // Deserialize the transform data
            // If you want to skip CRC check, call Unpack() without argument.
            int c = imgMsg->Unpack(1);
            if(c & igtl::MessageHeader::UNPACK_BODY) { // if CRC check is OK
                // Retrive the image data
                int size[3]; // image dimension
                float spacing[3]; // spacing (mm/pixel)
                int svsize[3]; // sub-volume size
                int svoffset[3]; // sub-volume offset
                int scalarType; // scalar type
                scalarType = imgMsg->GetScalarType();
                imgMsg->GetDimensions(size);
                imgMsg->GetSpacing(spacing);
                imgMsg->GetSubVolume(svsize, svoffset);
                /*
                std::cout << "Device Name : " << imgMsg->GetDeviceName() << std::endl;
                std::cout << "Scalar Type : " << scalarType << std::endl;
                std::cout << "Dimensions : ("
                << size[0] << ", " << size[1] << ", " << size[2] << ")" << std::endl;
                std::cout << "Spacing : ("
                << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << ")" << std::endl;
                std::cout << "Sub-Volume dimensions : ("
                << svsize[0] << ", " << svsize[1] << ", " << svsize[2] << ")" << std::endl;
                std::cout << "Sub-Volume offset : ("
                << svoffset[0] << ", " << svoffset[1] << ", " << svoffset[2] << ")" << std::endl << std::endl;
                */
                Image::pointer image = createFASTImageFromMessage(imgMsg, getMainDevice());
                DynamicData::pointer ptr = getOutputData<DynamicData >();
                try {
                    ptr->addFrame(image);
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    std::cout << "streamer has been deleted, stop" << std::endl;
                    break;
                }
                if(!mFirstFrameIsInserted) {
                    {
                        boost::lock_guard<boost::mutex> lock(mFirstFrameMutex);
                        mFirstFrameIsInserted = true;
                    }
                    mFirstFrameCondition.notify_one();
                }
                mNrOfFrames++;
            }
       }
    }
    mSocket->CloseSocket();
}

ImageIGTLinkStreamer::~ImageIGTLinkStreamer() {
    if(mStreamIsStarted) {
        if(thread->get_id() != boost::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}

ImageIGTLinkStreamer::ImageIGTLinkStreamer() {
    mStreamIsStarted = false;
    mIsModified = true;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mNrOfFrames = 0;
    mAddress = "";
    mPort = 0;
    mMaximumNrOfFramesSet = false;
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(ImageIGTLinkStreamer* streamer) {
    streamer->producerStream();
}

void ImageIGTLinkStreamer::execute() {
    getOutputData<DynamicData >()->setStreamer(mPtr.lock());
    if(mAddress == "" || mPort == 0) {
        throw Exception("Must call setConnectionAddress and setConnectionPort before executing the ImageIGTLinkStreamer.");
    }

    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    boost::unique_lock<boost::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

} // end namespace fast


