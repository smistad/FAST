#include "Tensor.hpp"
#include <FAST/Utility.hpp>
#include <FAST/Data/Access/OpenCLBufferAccess.hpp>

namespace fast {

void Tensor::create(std::unique_ptr<float[]> data, TensorShape shape) {
    if(shape.empty())
        throw Exception("Shape can't be empty");
    m_data = std::move(data);
    m_shape = shape;
    m_spacing = VectorXf::Ones(shape.getDimensions());
    mHostDataIsUpToDate = true;
    if(m_shape.getDimensions() >= 3) {
        const int width = m_shape[m_shape.getDimensions() - 2];
        const int height = m_shape[m_shape.getDimensions() - 3];
        mBoundingBox = BoundingBox(Vector3f(width, height, 1));
    }
}

void Tensor::create(TensorShape shape) {
    if(shape.empty())
        throw Exception("Shape can't be empty");
    if(shape.getUnknownDimensions() > 0)
        throw Exception("When creating a tensor, shape must be fully defined");
    m_data = make_uninitialized_unique<float[]>(shape.getTotalSize());
    m_spacing = VectorXf::Ones(shape.getDimensions());
    mHostDataIsUpToDate = true;
    if(m_shape.getDimensions() >= 3) {
        const int width = m_shape[m_shape.getDimensions() - 2];
        const int height = m_shape[m_shape.getDimensions() - 3];
        mBoundingBox = BoundingBox(Vector3f(width, height, 1));
    }
}

void Tensor::create(std::initializer_list<float> data) {
	if(data.size() == 0)
		throw Exception("Shape can't be empty");

	m_data = std::make_unique<float[]>(data.size());
	int i = 0;
	for(auto item : data) {
		m_data[i] = item;
		++i;
	}
	m_shape = TensorShape({ (int)data.size() });
    m_spacing = VectorXf::Ones(m_shape.getDimensions());
    mHostDataIsUpToDate = true;
    if(m_shape.getDimensions() >= 3) {
        const int width = m_shape[m_shape.getDimensions() - 2];
        const int height = m_shape[m_shape.getDimensions() - 3];
        mBoundingBox = BoundingBox(Vector3f(width, height, 1));
    }
}

void Tensor::expandDims(int position) {
	if(position < 0) { // append to end
		m_shape.addDimension(1);
	} else {
		m_shape.insertDimension(position, 1);
	}
	// TODO fix spacing
}

TensorShape Tensor::getShape() const {
    return m_shape;
}

TensorAccess::pointer Tensor::getAccess(accessType type) {
    if(!isInitialized())
        throw Exception("Tensor has not been initialized.");

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateHostData();
    if(type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    mHostDataIsUpToDate = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }
    return std::make_unique<TensorAccess>(getHostDataPointer(), m_shape, std::static_pointer_cast<Tensor>(mPtr.lock()));
}

void Tensor::free(ExecutionDevice::pointer device) {
    if(device->isHost()) {
        m_data.reset();
    } else {
        auto clDevice = std::dynamic_pointer_cast<OpenCLDevice>(device);
        delete mCLBuffers[clDevice];
        mCLBuffers.erase(clDevice);
        mCLBuffersIsUpToDate.erase(clDevice);
    }
}

void Tensor::freeAll() {
    m_data.reset();
    for(auto buffer : mCLBuffers) {
        delete buffer.second;
    }
    mCLBuffers.clear();
    mCLBuffersIsUpToDate.clear();
}

OpenCLBufferAccess::pointer Tensor::getOpenCLBufferAccess(accessType type, OpenCLDevice::pointer device) {
    if(!isInitialized())
        throw Exception("Tensor has not been initialized.");

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateOpenCLBufferData(device);
    if(type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    mCLBuffersIsUpToDate[device] = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

    // Now it is guaranteed that the data is on the device and that it is up to date
	auto accessObject = std::make_unique<OpenCLBufferAccess>(mCLBuffers[device],  std::dynamic_pointer_cast<DataObject>(mPtr.lock()));
	return std::move(accessObject);
}

bool Tensor::isInitialized() {
    return !m_shape.empty();
}

bool Tensor::hasAnyData() {
    return m_data.get() != nullptr || mCLBuffers.size() > 0;
}

void Tensor::updateOpenCLBufferData(OpenCLDevice::pointer device) {

    // If data exist on device and is up to date do nothing
    if (mCLBuffersIsUpToDate.count(device) > 0 && mCLBuffersIsUpToDate[device])
        return;

    bool updated = false;
    if(mCLBuffers.count(device) == 0) {
        // Data is not on device, create it
        unsigned int bufferSize = getShape().getTotalSize()*4;
        cl::Buffer * newBuffer = new cl::Buffer(
                device->getContext(),
                CL_MEM_READ_WRITE,
                bufferSize
        );

        if(hasAnyData()) {
            mCLBuffersIsUpToDate[device] = false;
        } else {
           mCLBuffersIsUpToDate[device] = true;
            updated = true;
        }
        mCLBuffers[device] = newBuffer;
    }

    // Find which data is up to date
    if(!mCLBuffersIsUpToDate[device]) {
        // Transfer host data to this device
        transferCLBufferFromHost(device);
        updated = true;
    }

    if(!updated)
        throw Exception("Data was not updated because no data was marked as up to date");

    mCLBuffersIsUpToDate[device] = true;
}

void Tensor::transferCLBufferFromHost(OpenCLDevice::pointer device) {
    std::size_t bufferSize = m_shape.getTotalSize()*4;
    device->getCommandQueue().enqueueWriteBuffer(*mCLBuffers[device],
        CL_TRUE, 0, bufferSize, getHostDataPointer());
}

void Tensor::transferCLBufferToHost(OpenCLDevice::pointer device) {
	if(!m_data) {
		// Must allocate memory for host data
        m_data = make_uninitialized_unique<float[]>(m_shape.getTotalSize());
	}
    std::size_t bufferSize = m_shape.getTotalSize()*4;
    device->getCommandQueue().enqueueReadBuffer(*mCLBuffers[device],
        CL_TRUE, 0, bufferSize, getHostDataPointer());
}

void Tensor::setAllDataToOutOfDate() {
    mHostDataIsUpToDate = false;
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(auto& it : mCLBuffersIsUpToDate) {
        it.second = false;
    }
}


void Tensor::updateHostData() {
    // It is the host data that has been modified, no need to update
    if(mHostDataIsUpToDate)
        return;

    bool updated = false;
    if(!m_data) {
        // Data is not initialized, do that first
        m_data = make_uninitialized_unique<float[]>(m_shape.getTotalSize());

        if(hasAnyData()) {
            mHostDataIsUpToDate = false;
        } else {
            mHostDataIsUpToDate = true;
            updated = true;
        }
    }

    for(auto& it : mCLBuffersIsUpToDate) {
        if(it.second == true) {
            // transfer from this device to host
            transferCLBufferToHost(it.first);
            updated = true;
            break;
        }
    }
    if(!updated)
        throw Exception(
                "Data was not updated because no data was marked as up to date");
}


void Tensor::setSpacing(VectorXf spacing) {
    if(!isInitialized())
        throw Exception("Tensor was not initialized");
    if(spacing.size() != m_shape.getDimensions())
        throw Exception("Spacing vector has different size than shape");
    m_spacing = spacing;
}

VectorXf Tensor::getSpacing() const {
    return m_spacing;
}

void Tensor::deleteDimension(int i) {
    if(m_shape.getDimensions() > i && m_shape[i] == 1) {
        m_shape.deleteDimension(i);
    } else {
        throw Exception("Invalid dimension to delete in tensor");
    }
}

BoundingBox Tensor::getTransformedBoundingBox() const {
    AffineTransformation::pointer T = SceneGraph::getAffineTransformationFromNode(getSceneGraphNode());

    // Add image spacing
    T->getTransform().scale(Vector3f(getSpacing().x(), getSpacing().y(), getSpacing().z()));

    return SpatialDataObject::getBoundingBox().getTransformedBoundingBox(T);
}

BoundingBox Tensor::getBoundingBox() const {
    // Add image spacing
    AffineTransformation::pointer T = AffineTransformation::New();
    T->getTransform().scale(Vector3f(getSpacing().x(), getSpacing().y(), getSpacing().z()));

    return SpatialDataObject::getBoundingBox().getTransformedBoundingBox(T);
}

float* Tensor::getHostDataPointer() {
    return m_data.get();
}

Tensor::~Tensor() {
	freeAll();
}

}