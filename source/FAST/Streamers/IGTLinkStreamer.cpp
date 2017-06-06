#include "IGTLinkStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/AffineTransformation.hpp"
#include <igtl/igtlOSUtil.h>
#include <igtl/igtlMessageHeader.h>
#include <igtl/igtlTransformMessage.h>
#include <igtl/igtlPositionMessage.h>
#include <igtl/igtlImageMessage.h>
#include <igtl/igtlStatusMessage.h>
#include <igtl/igtlStringMessage.h>

namespace fast {

void IGTLinkStreamer::setConnectionAddress(std::string address) {
    mAddress = address;
    mIsModified = true;
}

void IGTLinkStreamer::setConnectionPort(uint port) {
    mPort = port;
    mIsModified = true;
}

ProcessObjectPort IGTLinkStreamer::getOutputPort() {
	uint portID;
	if (mOutputPortDeviceNames.count("") == 0) {
		portID = getNrOfOutputPorts();
		createOutputPort<Image>(portID, OUTPUT_DYNAMIC);
		getOutputData<Image>(portID); // This initializes the output data
		mOutputPortDeviceNames[""] = portID;
	}
	else {
		portID = mOutputPortDeviceNames[""];
	}
	return ProcessObject::getOutputPort(portID);
}

void IGTLinkStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);
}

void IGTLinkStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
}

bool IGTLinkStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

uint IGTLinkStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

std::set<std::string> IGTLinkStreamer::getImageStreamNames() {
    return mImageStreamNames;
}

std::set<std::string> IGTLinkStreamer::getTransformStreamNames() {
    return mTransformStreamNames;
}

std::string IGTLinkStreamer::getStreamDescription(std::string streamName) {
    return mStreamDescriptions.at(streamName);
}

std::vector<std::string> IGTLinkStreamer::getActiveImageStreamNames() {
    std::vector<std::string> activeStreams;
    for(auto stream : mOutputPortDeviceNames) {
        if(mImageStreamNames.count(stream.first) > 0)
            activeStreams.push_back(stream.first);
    }
    return activeStreams;
}

std::vector<std::string> IGTLinkStreamer::getActiveTransformStreamNames() {
    std::vector<std::string> activeStreams;
    for(auto stream : mOutputPortDeviceNames) {
        if(mTransformStreamNames.count(stream.first) > 0)
            activeStreams.push_back(stream.first);
    }
    return activeStreams;
}

inline Image::pointer createFASTImageFromMessage(igtl::ImageMessage::Pointer message, ExecutionDevice::pointer device) {
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

    UniquePointer<float[]> spacing(new float[3]);
    UniquePointer<float[]> offset(new float[3]);
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

void IGTLinkStreamer::updateFirstFrameSetFlag() {
    // Check that all output ports have got their first frame
    bool allHaveGotData = true;
    for(uint i = 0; i < getNrOfOutputPorts(); i++) {
        DynamicData::pointer data = ProcessObject::getOutputPort(i).getData();
        if(data->getSize() == 0) {
            allHaveGotData = false;
            break;
        }
    }

    if(allHaveGotData) {
        {
            std::lock_guard<std::mutex> lock(mFirstFrameMutex);
            mFirstFrameIsInserted = true;
        }
        mFirstFrameCondition.notify_one();
    } else {
        reportInfo() << "ALL HAVE NOT GOT DATA" << Reporter::end();
    }
}

void IGTLinkStreamer::producerStream() {

    reportInfo() << "Connected to Open IGT Link server" << Reporter::end();;

    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();

    // Allocate a time stamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();
    uint statusMessageCounter = 0;

    while(true) {
        {
            std::unique_lock<std::mutex> lock(mStopMutex);
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
            //connectionLostSignal();
            mSocket->CloseSocket();
            break;
        }
        if(r != headerMsg->GetPackSize()) {
           continue;
        }

        // Deserialize the header
        headerMsg->Unpack();

        // Get time stamp
        headerMsg->GetTimeStamp(ts);

        reportInfo() << "Device name: " << headerMsg->GetDeviceName() << Reporter::end();

        unsigned long timestamp = round(ts->GetTimeStamp()*1000); // convert to milliseconds
        reportInfo() << "TIMESTAMP converted: " << timestamp << reportEnd();
        if(strcmp(headerMsg->GetDeviceType(), "TRANSFORM") == 0) {
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
            mSocket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize());
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
                DynamicData::pointer ptr;
                try {
                     ptr = getOutputDataFromDeviceName<AffineTransformation>(headerMsg->GetDeviceName());
                     ptr->setStreamer(mPtr.lock());
                } catch(Exception &e) {
                    reportInfo() << "Output port with device name " << headerMsg->GetDeviceName() << " not found" << Reporter::end();
                    continue;
                }
                try {
                    AffineTransformation::pointer T = AffineTransformation::New();
                    T->getTransform().matrix() = fastMatrix;
                    T->setCreationTimestamp(timestamp);
                    ptr->addFrame(T);
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end();
                    break;
                }
                if(!mFirstFrameIsInserted) {
                    updateFirstFrameSetFlag();
                }
                mNrOfFrames++;
            }
        } else if(strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0) {
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
            mSocket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

            // Deserialize the transform data
            // If you want to skip CRC check, call Unpack() without argument.
            int c = imgMsg->Unpack(1);
            if(c & igtl::MessageHeader::UNPACK_BODY) { // if CRC check is OK
                if(mOutputPortDeviceNames.count("") > 0) {
                    // If no specific output ports have been specified, choose this first one
                    mOutputPortDeviceNames[headerMsg->GetDeviceName()] = mOutputPortDeviceNames[""];
                    mOutputPortDeviceNames.erase("");
                }
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

                DynamicData::pointer ptr;
                try {
                     ptr = getOutputDataFromDeviceName<Image>(headerMsg->GetDeviceName());
                     ptr->setStreamer(mPtr.lock());
                } catch(Exception &e) {
                    reportInfo() << "Output port with device name " << headerMsg->GetDeviceName() << " not found" << Reporter::end();
                    continue;
                }
                try {
                    Image::pointer image = createFASTImageFromMessage(imgMsg, getMainDevice());
                    image->setCreationTimestamp(timestamp);
                    ptr->addFrame(image);
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end();
                    break;
                }
                if(!mFirstFrameIsInserted) {
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
            mSocket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize());
            if(statusMessageCounter > 3 && !mInFreezeMode) {
                reportInfo() << "3 STATUS MESSAGE received, freeze detected" << Reporter::end();
                mInFreezeMode = true;
                //freezeSignal();

                // If no frames has been inserted, stop
                if(!mFirstFrameIsInserted) {
					{
						std::lock_guard<std::mutex> lock(mFirstFrameMutex);
						mFirstFrameIsInserted = true;
					}
					mStop = true;
					mFirstFrameCondition.notify_one();
                }
            }
       } else {
           // Receive generic message
          igtl::MessageBase::Pointer message;
          message = igtl::MessageBase::New();
          message->SetMessageHeader(headerMsg);
          message->AllocatePack();

          // Receive transform data from the socket
          mSocket->Receive(message->GetPackBodyPointer(), message->GetPackBodySize());

          // Deserialize the transform data
          // If you want to skip CRC check, call Unpack() without argument.
          int c = message->Unpack();
       }
    }
    // Make sure we end the waiting thread if first frame has not been inserted
    {
        std::lock_guard<std::mutex> lock(mFirstFrameMutex);
        if(!mFirstFrameIsInserted)
            mFirstFrameIsInserted = true;
    }
    mFirstFrameCondition.notify_one();
    mSocket->CloseSocket();
    reportInfo() << "OpenIGTLink socket closed" << reportEnd();
}

IGTLinkStreamer::~IGTLinkStreamer() {
    if(mStreamIsStarted) {
        stop();
        if(thread->get_id() != std::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}

IGTLinkStreamer::IGTLinkStreamer() {
    mStreamIsStarted = false;
    mIsModified = true;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mStop = false;
    mNrOfFrames = 0;
    mAddress = "";
    mPort = 0;
    mMaximumNrOfFramesSet = false;
    mInFreezeMode = false;
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

void IGTLinkStreamer::stop() {
    std::unique_lock<std::mutex> lock(mStopMutex);
    mStop = true;
}

void IGTLinkStreamer::execute() {
    if(mAddress == "" || mPort == 0) {
        throw Exception("Must call setConnectionAddress and setConnectionPort before executing the IGTLinkStreamer.");
    }

    if(!mStreamIsStarted) {
        for(uint i = 0; i < getNrOfOutputPorts(); i++) {
            DynamicData::pointer data = ProcessObject::getOutputPort(i).getData();
            data->setMaximumNumberOfFrames(mMaximumNrOfFrames);
        }

        mSocket = igtl::ClientSocket::New();
        reportInfo() << "Trying to connect to Open IGT Link server " << mAddress << ":" << std::to_string(mPort) << Reporter::end();
        int r = mSocket->ConnectToServer(mAddress.c_str(), mPort);
        if(r != 0) {
            throw Exception("Failed to connect to Open IGT Link server " + mAddress + ":" + std::to_string(mPort));
        }

        mStreamIsStarted = true;
        thread = new std::thread(std::bind(&IGTLinkStreamer::producerStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
    {
        std::unique_lock<std::mutex> lock(mStopMutex);
        //if(!mStop)
        //    connectionEstablishedSignal(); // send signal
    }
}

} // end namespace fast
