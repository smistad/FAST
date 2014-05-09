#include "catch.hpp"
#include "Image.hpp"
#include "DeviceManager.hpp"
#include <ctime>
#include <cmath>
#include "HelperFunctions.hpp"

using namespace fast;

TEST_CASE("Create a 2D image on host", "[fast][image]") {
    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int nrOfComponents = 1;
    DataType type = TYPE_FLOAT;
    image->create2DImage(width, height, type, nrOfComponents, Host::New());

    CHECK(image->getWidth() == width);
    CHECK(image->getHeight() == height);
    CHECK(image->getDepth() == 1);
    CHECK(image->getNrOfComponents() == nrOfComponents);
    CHECK(image->getDataType() == type);
    CHECK(image->getDimensions() == 2);
}

TEST_CASE("Create a 3D image on host", "[fast][image]") {
    Image::pointer image = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int depth = 45;
    unsigned int nrOfComponents = 2;
    DataType type = TYPE_INT8;
    image->create3DImage(width, height, depth, type, nrOfComponents, Host::New());

    CHECK(image->getWidth() == width);
    CHECK(image->getHeight() == height);
    CHECK(image->getDepth() == depth);
    CHECK(image->getNrOfComponents() == nrOfComponents);
    CHECK(image->getDataType() == type);
    CHECK(image->getDimensions() == 3);
}

TEST_CASE("Create a 2D image on an OpenCL device", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;
    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            Image::pointer image = Image::New();
            CHECK_NOTHROW(image->create2DImage(width, height, (DataType)typeNr, nrOfComponents, device));
        }
    }
}

TEST_CASE("Create a 3D image on an OpenCL device", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int depth = 45;
    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            Image::pointer image = Image::New();
            CHECK_NOTHROW(image->create3DImage(width, height, depth, (DataType)typeNr, nrOfComponents, device));
        }
    }
}

void* allocateRandomData(unsigned int nrOfVoxels, DataType type) {
    srand(time(NULL));
    switch(type) {
    case TYPE_FLOAT:
    {
        float* data = new float[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = (float)rand() / RAND_MAX;
        return (void*)data;
    }
        break;
    case TYPE_INT8:
    {
        char* data = new char[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255 - 128;
        return (void*)data;
    }
        break;
    case TYPE_UINT8:
    {
        uchar* data = new uchar[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255;
        return (void*)data;
    }
        break;
    case TYPE_INT16:
    {
        short* data = new short[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255 - 128;
        return (void*)data;
    }
        break;
    case TYPE_UINT16:
    {
        ushort* data = new ushort[nrOfVoxels];
        for(unsigned int i = 0; i < nrOfVoxels; i++)
            data[i] = rand() % 255;
        return (void*)data;
    }
        break;
    }
    return NULL;
}

bool compareDataArrays(void* data1, void* data2, unsigned int nrOfVoxels, DataType type) {
    bool success = true;
    switch(type) {
        fastSwitchTypeMacro(
        FAST_TYPE* data1c = (FAST_TYPE*)data1;
        FAST_TYPE* data2c = (FAST_TYPE*)data2;
        for(unsigned int i = 0; i < nrOfVoxels; i++) {
            if(data1c[i] != data2c[i]) {
                success = false;
                break;
            }
        }
        );
    }
    return success;
}

bool compareBufferWithDataArray(cl::Buffer buffer, OpenCLDevice::pointer device, void* data, unsigned int nrOfVoxels, DataType type) {
    // First, transfer data from buffer
    unsigned int elementSize;
    void* bufferData;
    switch(type) {
        fastSwitchTypeMacro(
            elementSize = sizeof(FAST_TYPE);
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadBuffer(buffer, CL_TRUE, 0, nrOfVoxels*elementSize, bufferData);

    return compareDataArrays(bufferData, data, nrOfVoxels, type);
}

bool compareImage2DWithDataArray(
        cl::Image2D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int nrOfComponents,
        DataType type) {
    // First, transfer data from image
    void* bufferData;
    unsigned int nrOfVoxels = width*height*nrOfComponents;
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, oul::createOrigoRegion(), oul::createRegion(width, height, 1), 0, 0, bufferData);

    return compareDataArrays(bufferData, data, nrOfVoxels, type);
}

bool compareImage3DWithDataArray(
        cl::Image3D image,
        OpenCLDevice::pointer device,
        void* data,
        unsigned int width,
        unsigned int height,
        unsigned int depth,
        unsigned int nrOfComponents,
        DataType type) {
    // First, transfer data from image
    void* bufferData;
    unsigned int nrOfVoxels = width*height*depth*nrOfComponents;
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, oul::createOrigoRegion(), oul::createRegion(width, height, depth), 0, 0, bufferData);

    return compareDataArrays(bufferData, data, nrOfVoxels, type);
}

TEST_CASE("Create a 2D image on an OpenCL device with data", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;
    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;
            void* data = allocateRandomData(width*height*nrOfComponents, (DataType)typeNr);
            Image::pointer image = Image::New();
            CHECK_NOTHROW(image->create2DImage(width, height, type, nrOfComponents, device, data));
            ImageAccess access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access.get(), width*height*nrOfComponents, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 3D image on an OpenCL device with data", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 40;
    unsigned int height = 64;
    unsigned int depth = 64;
    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);
            Image::pointer image = Image::New();
            CHECK_NOTHROW(image->create3DImage(width, height, depth, type, nrOfComponents, device, data));
            ImageAccess access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access.get(), width*height*depth*nrOfComponents, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 2D image on host with input data", "[fast][image]") {
    unsigned int width = 256;
    unsigned int height = 512;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, Host::New(), data);

            ImageAccess access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access.get(), width*height*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 3D image on host with input data", "[fast][image]") {
    unsigned int width = 64;
    unsigned int height = 128;
    unsigned int depth = 45;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::New(), data);

            ImageAccess access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access.get(), width*height*depth*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 2D image on host and request access to OpenCL buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, Host::New(), data);

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}
TEST_CASE("Create a 3D image on host and request access to OpenCL buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 40;
    unsigned int height = 64;
    unsigned int depth = 64;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::New(), data);

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*depth*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 2D image on host and request access to OpenCL Image", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        // Skip 3 components because an OpenCL image can't have 3 channels
        if(nrOfComponents == 3)
            continue;
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, Host::New(), data);

            OpenCLImageAccess2D access = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access.get();
            CHECK(compareImage2DWithDataArray(*clImage, device, data, width, height, nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 3D image on host and request access to OpenCL Image", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 40;
    unsigned int height = 64;
    unsigned int depth = 64;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        // Skip 3 components because an OpenCL image can't have 3 channels
        if(nrOfComponents == 3)
            continue;
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::New(), data);

            OpenCLImageAccess3D access = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access.get();
            CHECK(compareImage3DWithDataArray(*clImage, device, data, width, height, depth, nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 2D image on a CL device and request access to OpenCL buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 256;
    unsigned int height = 512;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, device, data);

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 3D image on CL device and request access to OpenCL buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 40;
    unsigned int height = 64;
    unsigned int depth = 64;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*depth*nrOfComponents, type) == true);

            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create an image twice", "[fast][image]") {
    Image::pointer image = Image::New();

    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());
    CHECK_THROWS(image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New()));
}
