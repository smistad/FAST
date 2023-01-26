#include "OpenIGTLinkStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/SimpleDataObject.hpp"
#include <igtl/igtlOSUtil.h>
#include <igtl/igtlMessageHeader.h>
#include <igtl/igtlTransformMessage.h>
#include <igtl/igtlPositionMessage.h>
#include <igtl/igtlImageMessage.h>
#include <igtl/igtlStatusMessage.h>
#include <igtl/igtlStringMessage.h>
#include <igtl/igtlClientSocket.h>
#include <chrono>
#include <string>

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
    if(getNrOfOutputPorts() == 0) {
        portID = getOutputPortNumber("");
    }
	return Streamer::getOutputPort(portID);
}

uint OpenIGTLinkStreamer::getOutputPortNumber(std::string deviceName) {
    if(mOutputPortDeviceNames.count(deviceName) == 0) {
        uint portID = getNrOfOutputPorts();
        createOutputPort(portID);
        mOutputPortDeviceNames[deviceName] = portID;
    }
    return mOutputPortDeviceNames[deviceName];
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

    Image::pointer image;
    if(depth == 1) {
        image = Image::create(width, height, type, message->GetNumComponents(), device, data);
    } else {
        image = Image::create(width, height, depth, type, message->GetNumComponents(), device, data);
    }

    auto spacing = std::make_unique<float[]>(3);
    auto offset = std::make_unique<float[]>(3);
    message->GetSpacing(spacing.get());
    message->GetOrigin(offset.get());
    igtl::Matrix4x4 matrix;
    message->GetMatrix(matrix);
    image->setSpacing(Vector3f(spacing[0], spacing[1], spacing[2]));
    auto T = Affine3f::Identity();
    T.translation() = Vector3f(offset[0], offset[1], offset[2]);
    Matrix3f fastMatrix;
    for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
        fastMatrix(i,j) = matrix[i][j];
    }}
    T.linear() = fastMatrix;
    image->getSceneGraphNode()->setTransform(T);


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
        bool timeout = false;
        int r = mSocketWrapper->socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize(), timeout);
        if(r == 0 || timeout) {
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

        uint64_t timestamp = round(ts->GetTimeStamp()*1000); // convert to milliseconds
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
            bool timeout = false;
            int r = mSocketWrapper->socket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize(), timeout);
			if(r == 0 || timeout) {
				//connectionLostSignal();
				mSocketWrapper->socket->CloseSocket();
				break;
			}

            // Deserialize the transform data
            // If you want to skip CRC check, call Unpack() without argument.
            int c = transMsg->Unpack(1);

            if(c & igtl::MessageHeader::UNPACK_BODY) { // if CRC check is OK
                // Retrive the transform data
                igtl::Matrix4x4 matrix;
                transMsg->GetMatrix(matrix);
                Affine3f fastTransform;
                for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    fastTransform.matrix()(i,j) = matrix[i][j];
                }}
                reportInfo() << fastTransform.matrix() << Reporter::end();

                try {
                    auto T = Transform::create(fastTransform);
                    T->setCreationTimestamp(timestamp);
                    addTimestamp(timestamp);
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
            bool timeout = false;
            int r = mSocketWrapper->socket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize(), timeout);
			if(r == 0 || timeout) {
				//connectionLostSignal();
				mSocketWrapper->socket->CloseSocket();
				break;
			}

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
                    auto image = createFASTImageFromMessage(imgMsg, getMainDevice());
					image->setCreationTimestamp(timestamp);
					addTimestamp(timestamp);
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
            bool timeout = false;
            int r = mSocketWrapper->socket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize(), timeout);
            if (r == 0 || timeout) {
                //connectionLostSignal();
                mSocketWrapper->socket->CloseSocket();
                break;
            }


            if (statusMessageCounter > 3 && !mInFreezeMode) {
                reportInfo() << "3 STATUS MESSAGE received, freeze detected" << Reporter::end();
                mInFreezeMode = true;
                //freezeSignal();

                // If no frames has been inserted, stop
                frameAdded();
            }
        } else if(strcmp(headerMsg->GetDeviceType(), "STRING") == 0 && !ignore) {
            // Receive generic message
            igtl::StringMessage::Pointer stringMsg;
            stringMsg = igtl::StringMessage::New();
            stringMsg->SetMessageHeader(headerMsg);
            stringMsg->AllocatePack();

            // Receive transform data from the socket
            bool timeout = false;
            int r = mSocketWrapper->socket->Receive(stringMsg->GetPackBodyPointer(), stringMsg->GetPackBodySize(), timeout);
            if (r == 0 || timeout) {
                //connectionLostSignal();
                mSocketWrapper->socket->CloseSocket();
                break;
            }

            stringMsg->Unpack();
            auto message = stringMsg->GetString();

            auto fastString = String::create(message);
            fastString->setCreationTimestamp(timestamp);

            addTimestamp(timestamp);
            addOutputData(mOutputPortDeviceNames[deviceName], fastString);
       } else {
           // Receive generic message
          igtl::MessageBase::Pointer message;
          message = igtl::MessageBase::New();
          message->SetMessageHeader(headerMsg);
          message->AllocatePack();

          // Receive transform data from the socket
          bool timeout = false;
          int r = mSocketWrapper->socket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize(), timeout);
		  if(r == 0 || timeout) {
				//connectionLostSignal();
				mSocketWrapper->socket->CloseSocket();
				break;
	     }



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

OpenIGTLinkStreamer::OpenIGTLinkStreamer(std::string ipAddress, int port) {
    mIsModified = true;
    mNrOfFrames = 0;
    mMaximumNrOfFramesSet = false;
    mInFreezeMode = false;

    createStringAttribute("address", "Connection address", "Connection address", ipAddress);
    createIntegerAttribute("port", "Connection port", "Connection port", port);

    setConnectionAddress(ipAddress);
    setConnectionPort(port);
    setStreamingMode(StreamingMode::NewestFrameOnly);
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

void OpenIGTLinkStreamer::addTimestamp(uint64_t timestamp) {
    m_timestamps.push_back(timestamp);
    if(m_timestamps.size() == 10)
        m_timestamps.pop_front();
}

float OpenIGTLinkStreamer::getCurrentFramerate() {
    if(m_timestamps.empty())
        return -1;
    uint64_t sum = 0;
    uint64_t previous = m_timestamps[0];
    int counter = 0;
    for(int i = 1; i < m_timestamps.size(); ++i) {
        uint64_t duration = m_timestamps[i] - previous;
        previous = m_timestamps[i];
        sum += duration;
        ++counter;
    }
    return 1000.0f/((float)sum / counter); // Timestamps are i milliseconds. get framerate in per second
}

DataChannel::pointer OpenIGTLinkStreamer::getOutputPort(std::string deviceName) {
    uint portID;
    if(mOutputPortDeviceNames.count(deviceName) == 0) {
        portID = getNrOfOutputPorts();
        createOutputPort(portID);
        mOutputPortDeviceNames[deviceName] = portID;
    } else {
        portID = mOutputPortDeviceNames[deviceName];
    }
    return Streamer::getOutputPort(portID);
}

uint OpenIGTLinkStreamer::createOutputPortForDevice(std::string deviceName) {
    uint portID;
    if(mOutputPortDeviceNames.count(deviceName) == 0) {
        portID = getNrOfOutputPorts();
        createOutputPort(portID);
        mOutputPortDeviceNames[deviceName] = portID;
    } else {
        portID = mOutputPortDeviceNames[deviceName];
    }
    return portID;
}
} // end namespace fast
