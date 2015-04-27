#include "TransformIGTLinkStreamer.hpp"
#include "DynamicData.hpp"
#include "LinearTransformation.hpp"
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>

#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlTransformMessage.h"
#include "igtlPositionMessage.h"
#include "igtlImageMessage.h"
#include "igtlStatusMessage.h"
#include "igtlStringMessage.h"

namespace fast {


inline int ReceiveString(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
{

  //std::cerr << "Receiving STRING data type." << std::endl;

  // Create a message buffer to receive transform data
  igtl::StringMessage::Pointer stringMsg;
  stringMsg = igtl::StringMessage::New();
  stringMsg->SetMessageHeader(header);
  stringMsg->AllocatePack();

  // Receive transform data from the socket
  socket->Receive(stringMsg->GetPackBodyPointer(), stringMsg->GetPackBodySize());

  // Deserialize the transform data
  // If you want to skip CRC check, call Unpack() without argument.
  int c = stringMsg->Unpack(1);

  if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
    //std::cerr << "Encoding: " << stringMsg->GetEncoding() << "; "
    //          << "String: " << stringMsg->GetString() << std::endl << std::endl;
    }

  return 1;
}

inline int ReceivePosition(igtl::Socket * socket, igtl::MessageHeader::Pointer& header) {
    std::cerr << "Receiving POSITION data type." << std::endl;
    // Create a message buffer to receive transform data
    igtl::PositionMessage::Pointer positionMsg;
    positionMsg = igtl::PositionMessage::New();
    positionMsg->SetMessageHeader(header);
    positionMsg->AllocatePack();
    // Receive position position data from the socket
    socket->Receive(positionMsg->GetPackBodyPointer(), positionMsg->GetPackBodySize());
    // Deserialize the transform data
    // If you want to skip CRC check, call Unpack() without argument.
    int c = positionMsg->Unpack(1);
    if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
    // Retrive the transform data
    float position[3];
    float quaternion[4];
    positionMsg->GetPosition(position);
    positionMsg->GetQuaternion(quaternion);
    std::cerr << "position = (" << position[0] << ", " << position[1] << ", " << position[2] << ")" << std::endl;
    std::cerr << "quaternion = (" << quaternion[0] << ", " << quaternion[1] << ", "
    << quaternion[2] << ", " << quaternion[3] << ")" << std::endl << std::endl;
    return 1;
    }
    return 0;
}

inline int ReceiveStatus(igtl::Socket * socket, igtl::MessageHeader::Pointer& header) {
    std::cerr << "Receiving STATUS data type." << std::endl;
    // Create a message buffer to receive transform data
    igtl::StatusMessage::Pointer statusMsg;
    statusMsg = igtl::StatusMessage::New();
    statusMsg->SetMessageHeader(header);
    statusMsg->AllocatePack();
    // Receive transform data from the socket
    socket->Receive(statusMsg->GetPackBodyPointer(), statusMsg->GetPackBodySize());
    // Deserialize the transform data
    // If you want to skip CRC check, call Unpack() without argument.
    int c = statusMsg->Unpack(1);
    if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
    std::cerr << "========== STATUS ==========" << std::endl;
    std::cerr << " Code : " << statusMsg->GetCode() << std::endl;
    std::cerr << " SubCode : " << statusMsg->GetSubCode() << std::endl;
    std::cerr << " Error Name: " << statusMsg->GetErrorName() << std::endl;
    std::cerr << " Status : " << statusMsg->GetStatusString() << std::endl;
    std::cerr << "============================" << std::endl << std::endl;
    }
    return 0;
}

void TransformIGTLinkStreamer::setConnectionAddress(std::string address) {
    mAddress = address;
    mIsModified = true;
}

void TransformIGTLinkStreamer::setConnectionPort(uint port) {
    mPort = port;
    mIsModified = true;
}

void TransformIGTLinkStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);}

void TransformIGTLinkStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    DynamicData<LinearTransformation>::pointer data = getOutputData<DynamicData<LinearTransformation> >();
    data->setMaximumNumberOfFrames(nrOfFrames);
}

bool TransformIGTLinkStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

uint TransformIGTLinkStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}


void TransformIGTLinkStreamer::producerStream() {
    mSocket = igtl::ClientSocket::New();
    std::cout << "Trying to connect to Open IGT Link server " << mAddress << ":" << boost::lexical_cast<std::string>(mPort) << std::endl;;
    //mSocket->SetTimeout(3000); // try to connect for 3 seconds
    int r = mSocket->ConnectToServer(mAddress.c_str(), mPort);
    if(r != 0) {
       throw Exception("Cannot connect to the Open IGT Link server.");
    }
    std::cout << "Connected to Open IGT Link server" << std::endl;;

    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();

    // Allocate a time stamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();

    while(true) {
        {
            boost::unique_lock<boost::mutex> lock(mStopMutex);
            if(mStop) {
                mStreamIsStarted = false;
                mFirstFrameIsInserted = false;
                mHasReachedEnd = false;
                break;
            }
        }

        // Initialize receive buffer
        headerMsg->InitPack();

        // Receive generic header from the socket
        int r = mSocket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
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
        std::cout << "Device type: " << headerMsg->GetDeviceType() << std::endl;
        std::cout << "Device name: " << headerMsg->GetDeviceName() << std::endl;
        if(strcmp(headerMsg->GetDeviceType(), "STATUS") == 0) {
            ReceiveStatus(mSocket, headerMsg);
        } else if(strcmp(headerMsg->GetDeviceType(), "STRING") == 0) {
            ReceiveString(mSocket, headerMsg);
        } else if(strcmp(headerMsg->GetDeviceType(), "TRANSFORM") == 0) {
            // TODO get transform packet and put into dynamic object
            igtl::TransformMessage::Pointer transMsg;
            transMsg = igtl::TransformMessage::New();
            transMsg->SetMessageHeader(headerMsg);
            transMsg->AllocatePack();
            // Receive transform data from the socket
            mSocket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize());
            // Deserialize the transform data
            // If you want to skip CRC check, call Unpack() without argument.
            int c = transMsg->Unpack(1);
            if(mTransformName != "") {
                // Check that name is correct
                if(mTransformName != headerMsg->GetDeviceName())
                    continue;
            }
            if(c & igtl::MessageHeader::UNPACK_BODY) { // if CRC check is OK
                // Retrive the transform data
                igtl::Matrix4x4 matrix;
                transMsg->GetMatrix(matrix);
                Matrix4f fastMatrix;
                for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    fastMatrix(i,j) = matrix[i][j];
                }}
                igtl::PrintMatrix(matrix);
                Eigen::Transform<float, 3, Eigen::Affine> transformMatrix;
                transformMatrix.matrix() = fastMatrix;
                DynamicData<LinearTransformation>::pointer ptr = getOutputData<DynamicData<LinearTransformation> >();
                LinearTransformation::pointer T = LinearTransformation::New();
                T->setTransform(transformMatrix);
                try {
                    std::cout << "Adding LinearTransformation to stream.." << std::endl;
                    ptr->addFrame(T);
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
        } else if(strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0) {
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
       }
    }
    mSocket->CloseSocket();
}

TransformIGTLinkStreamer::~TransformIGTLinkStreamer() {
    if(mStreamIsStarted) {
        stop();
        if(thread->get_id() != boost::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}

TransformIGTLinkStreamer::TransformIGTLinkStreamer() {
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
    mTransformName = "";
}

/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(TransformIGTLinkStreamer* streamer) {
    streamer->producerStream();
}

void TransformIGTLinkStreamer::stop() {
    boost::unique_lock<boost::mutex> lock(mStopMutex);
    mStop = true;
}

void TransformIGTLinkStreamer::execute() {
    getOutputData<DynamicData<LinearTransformation> >()->setStreamer(mPtr.lock());
    if(mAddress == "" || mPort == 0) {
        throw Exception("Must call setConnectionAddress and setConnectionPort before executing the TransformIGTLinkStreamer.");
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

void TransformIGTLinkStreamer::setTransformName(std::string transformName) {
    mTransformName = transformName;
}

} // end namespace fast


