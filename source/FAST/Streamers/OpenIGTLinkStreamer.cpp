#include "OpenIGTLinkStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/AffineTransformation.hpp"
#include <igtl/igtlOSUtil.h>
#include <igtl/igtlMessageHeader.h>
#include <igtl/igtlTransformMessage.h>
#include <igtl/igtlPositionMessage.h>
#include <igtl/igtlImageMessage.h>
#include <igtl/igtlStatusMessage.h>
#include <igtl/igtlStringMessage.h>
#include <igtl/igtlClientSocket.h>
#include <chrono>

namespace fast {

	class IGTLSocketWrapper {
	public:
		IGTLSocketWrapper(igtl::ClientSocket::Pointer socket) : socket(socket) {};
		igtl::ClientSocket::Pointer socket;
	};

void OpenIGTLinkStreamer::setConnectionAddress(std::string address) {
    mAddress = address;
    mIsModified = true;
}

void OpenIGTLinkStreamer::setConnectionPort(uint port) {
    mPort = port;
    mIsModified = true;
}

DataChannel::pointer OpenIGTLinkStreamer::getOutputPort(uint portID) {
	if (mOutputPortDeviceNames.count("") == 0) {
		portID = getNrOfOutputPorts();
		createOutputPort<Image>(portID);
		getOutputData<Image>(portID); // This initializes the output data
		mOutputPortDeviceNames[""] = portID;
	}
	else {
		portID = mOutputPortDeviceNames[""];
	}
	return ProcessObject::getOutputPort(portID);
}

uint OpenIGTLinkStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

std::set<std::string> OpenIGTLinkStreamer::getImageStreamNames() {
    return mImageStreamNames;
}

std::set<std::string> OpenIGTLinkStreamer::getTransformStreamNames() {
    return mTransformStreamNames;
}

std::string OpenIGTLinkStreamer::getStreamDescription(std::string streamName) {
    return mStreamDescriptions.at(streamName);
}

std::vector<std::string> OpenIGTLinkStreamer::getActiveImageStreamNames() {
    std::vector<std::string> activeStreams;
    for(auto stream : mOutputPortDeviceNames) {
        if(mImageStreamNames.count(stream.first) > 0)
            activeStreams.push_back(stream.first);
    }
    return activeStreams;
}

std::vector<std::string> OpenIGTLinkStreamer::getActiveTransformStreamNames() {
    std::vector<std::string> activeStreams;
    for(auto stream : mOutputPortDeviceNames) {
        if(mTransformStreamNames.count(stream.first) > 0)
            activeStreams.push_back(stream.first);
    }
    return activeStreams;
}

static Image::pointer createFASTImageFromMessage(igtl::ImageMessage::Pointer message, ExecutionDevice::pointer device) {
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
        image->create(width, height, type, message->GetNumComponents(), device, data);
    } else {
        image->create(width, height, depth, type, message->GetNumComponents(), device, data);
    }

    auto spacing = std::make_unique<float[]>(3);
    auto offset = std::make_unique<float[]>(3);
    message->GetSpacing(spacing.get());
    message->GetOrigin(offset.get());
    igtl::Matrix4x4 matrix;
    message->GetMatrix(matrix);
    image->setSpacing(Vector3f(spacing[0], spacing[1], spacing[2]));
    AffineTransformation::pointer T = AffineTransformation::New();
    T->getTransform().translation() = Vector3f(offset[0], offset[1], offset[2]);
    Matrix3f fastMatrix;
    for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
        fastMatrix(i,j) = matrix[i][j];
    }}
    T->getTransform().linear() = fastMatrix;
    image->getSceneGraphNode()->setTransformation(T);


    return image;
}

void OpenIGTLinkStreamer::updateFirstFrameSetFlag() {
    // Check that all output ports have got their first frame
    bool allHaveGotData = true;
    for(auto port : mOutputConnections) {
        for(auto output : port.second) {
            if(output.lock()->getSize() == 0)
                allHaveGotData = false;
        }
    }

    if(allHaveGotData) {
        frameAdded();
    } else {
        reportInfo() << "ALL HAVE NOT GOT DATA" << Reporter::end();
    }
}

void OpenIGTLinkStreamer::generateStream() {

    reportInfo() << "Connected to Open IGT Link server" << Reporter::end();;

    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();

    // Allocate a time stamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();
    uint statusMessageCounter = 0;

    auto start = std::chrono::high_resolution_clock::now();

    while(true) {
        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }

        // Initialize receive buffer
        headerMsg->InitPack();

        // Receive generic header from the socket
        int r = mSocketWrapper->socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
        if(r == 0) {
            //connectionLostSignal();
            mSocketWrapper->socket->CloseSocket();
            break;
        }
        if(r != headerMsg->GetPackSize()) {
           continue;
        }

        // Deserialize the header
        headerMsg->Unpack();

        // Get time stamp
        headerMsg->GetTimeStamp(ts);

        std::string deviceName = headerMsg->GetDeviceName();
        reportInfo() << "Device name: " << deviceName << Reporter::end();
        bool ignore = false;
        if(mOutputPortDeviceNames.count(deviceName) == 0) {
            if(mOutputPortDeviceNames.count("") > 0 && strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0) {
                // If no specific output ports have been specified, choose this first one
                mOutputPortDeviceNames[headerMsg->GetDeviceName()] = mOutputPortDeviceNames[""];
                mOutputPortDeviceNames.erase("");
            } else {
                // Ignore this device stream if it doesn't exist
                ignore = true;
            }
        }

        //unsigned long timestamp = round(ts->GetTimeStamp()*1000); // convert to milliseconds
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = now - start;
        uint64_t timestamp = duration.count();
        reportInfo() << "TIMESTAMP converted: " << timestamp << reportEnd();
        if(strcmp(headerMsg->GetDeviceType(), "TRANSFORM") == 0 && !ignore) {
            mTransformStreamNames.insert(headerMsg->GetDeviceName());
            mStreamDescriptions[headerMsg->GetDeviceName()] = "Transform";
            if(mInFreezeMode) {
                //unfreezeSignal();
                mInFreezeMode = false;
            }
            statusMessageCounter = 0;
            igtl::TransformMessage::Pointer transMsg;
            transMsg = igtl::TransformMessage::New();
            transMsg->SetMessageHeader(headerMsg);
            transMsg->AllocatePack();
            // Receive transform data from the socket
            mSocketWrapper->socket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize());
            // Deserialize the transform data
            // If you want to skip CRC check, call Unpack() without argument.
            int c = transMsg->Unpack(1);

            if(c & igtl::MessageHeader::UNPACK_BODY) { // if CRC check is OK
                // Retrive the transform data
                igtl::Matrix4x4 matrix;
                transMsg->GetMatrix(matrix);
                Matrix4f fastMatrix;
                for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    fastMatrix(i,j) = matrix[i][j];
                }}
                reportInfo() << fastMatrix << Reporter::end();

                try {
                    AffineTransformation::pointer T = AffineTransformation::New();
                    T->getTransform().matrix() = fastMatrix;
                    T->setCreationTimestamp(timestamp);
                    addOutputData(mOutputPortDeviceNames[deviceName], T);
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end();
                    break;
                }
                if(!m_firstFrameIsInserted) {
                    updateFirstFrameSetFlag();
                }
                mNrOfFrames++;
            }
        } else if(strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0 && !ignore) {
            mImageStreamNames.insert(headerMsg->GetDeviceName());
            if(mInFreezeMode) {
                //unfreezeSignal();
                mInFreezeMode = false;
            }
            statusMessageCounter = 0;
            reportInfo() << "Receiving IMAGE data type from device " << headerMsg->GetDeviceName() << Reporter::end();

            // Create a message buffer to receive transform data
            igtl::ImageMessage::Pointer imgMsg;
            imgMsg = igtl::ImageMessage::New();
            imgMsg->SetMessageHeader(headerMsg);
            imgMsg->AllocatePack();

            // Receive transform data from the socket
            mSocketWrapper->socket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

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

                std::string description = "";
                if(size[2] == 1) {
                    description = "2D, " + std::to_string(size[0]) + "x" + std::to_string(size[1]);
                } else {
                    description = "3D, " + std::to_string(size[0]) + "x" + std::to_string(size[1]) + "x" + std::to_string(size[2]);
                }
                description += ", " + std::to_string(imgMsg->GetNumComponents()) + " channels, " + std::to_string(imgMsg->GetScalarSize()*8) + "bit";
                mStreamDescriptions[headerMsg->GetDeviceName()] = description;

                try {
                    Image::pointer image = createFASTImageFromMessage(imgMsg, getMainDevice());
                    image->setCreationTimestamp(timestamp);
                    addOutputData(mOutputPortDeviceNames[deviceName], image);
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end();
                    break;
                }
                if(!m_firstFrameIsInserted) {
                    updateFirstFrameSetFlag();
                }
                mNrOfFrames++;
            }
        } else if(strcmp(headerMsg->GetDeviceType(), "STATUS") == 0) {
            ++statusMessageCounter;
            reportInfo() << "STATUS MESSAGE recieved" << Reporter::end();
            // Receive generic message
            igtl::MessageBase::Pointer message;
            message = igtl::MessageBase::New();
            message->SetMessageHeader(headerMsg);
            message->AllocatePack();

            // Receive transform data from the socket
            mSocketWrapper->socket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize());
            if(statusMessageCounter > 3 && !mInFreezeMode) {
                reportInfo() << "3 STATUS MESSAGE received, freeze detected" << Reporter::end();
                mInFreezeMode = true;
                //freezeSignal();

                // If no frames has been inserted, stop
                frameAdded();
            }
       } else {
           // Receive generic message
          igtl::MessageBase::Pointer message;
          message = igtl::MessageBase::New();
          message->SetMessageHeader(headerMsg);
          message->AllocatePack();

          // Receive transform data from the socket
          mSocketWrapper->socket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize());

          // Deserialize the transform data
          // If you want to skip CRC check, call Unpack() without argument.
          int c = message->Unpack();
       }
    }
    // Make sure we end the waiting thread if first frame has not been inserted
    frameAdded();
    mSocketWrapper->socket->CloseSocket();
    reportInfo() << "OpenIGTLink socket closed" << reportEnd();
}

OpenIGTLinkStreamer::~OpenIGTLinkStreamer() {
    stop();
    if(m_streamIsStarted) {
		delete mSocketWrapper;
    }
}

void OpenIGTLinkStreamer::loadAttributes() {
    setConnectionAddress(getStringAttribute("address"));
    setConnectionPort(getIntegerAttribute("port"));
}

OpenIGTLinkStreamer::OpenIGTLinkStreamer() {
    mIsModified = true;
    mNrOfFrames = 0;
    mAddress = "localhost";
    mPort = 18944;
    mMaximumNrOfFramesSet = false;
    mInFreezeMode = false;

    createStringAttribute("address", "Connection address", "Connection address", mAddress);
    createIntegerAttribute("port", "Connection port", "Connection port", mPort);
}

void OpenIGTLinkStreamer::execute() {

    if(!m_streamIsStarted) {

		mSocketWrapper = new IGTLSocketWrapper(igtl::ClientSocket::New());
        reportInfo() << "Trying to connect to Open IGT Link server " << mAddress << ":" << std::to_string(mPort) << Reporter::end();
        int r = mSocketWrapper->socket->ConnectToServer(mAddress.c_str(), mPort);
        if(r != 0) {
            throw Exception("Failed to connect to Open IGT Link server " + mAddress + ":" + std::to_string(mPort));
        }

        m_streamIsStarted = true;
        m_thread = std::make_unique<std::thread>(std::bind(&OpenIGTLinkStreamer::generateStream, this));
    }

    // Wait here for first frame
    waitForFirstFrame();
}

} // end namespace fast
