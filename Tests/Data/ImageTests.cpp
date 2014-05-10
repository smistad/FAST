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
                /*
                std::cout << i << std::endl;
                std::cout << data1c[i] << std::endl;
                std::cout << data2c[i] << std::endl;
                */
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

// Remove padding from a 3 channel data array created by padData
template <class T>
void * removePadding(T * data, unsigned int size) {
     T * newData = new T[size*3];
    for(unsigned int i = 0; i < size; i++) {
        newData[i*3] = data[i*4];
        newData[i*3+1] = data[i*4+1];
        newData[i*3+2] = data[i*4+2];
    }
    return (void *)newData;
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
    unsigned int nrOfVoxels = width*height;
    if(nrOfComponents == 3) {
        nrOfVoxels *= 4;
    } else {
        nrOfVoxels *= nrOfComponents;
    }
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, oul::createOrigoRegion(), oul::createRegion(width, height, 1), 0, 0, bufferData);

    if(nrOfComponents == 3) {
        // Since OpenCL images doesn't support 3 channels, 4 channel image is used and we must remove the padding
        switch(type) {
            fastSwitchTypeMacro(
                void * newbufferData = removePadding((FAST_TYPE*)bufferData, width*height);
                deleteArray(bufferData, type);
                bufferData = newbufferData;
            );
        }
    }

    return compareDataArrays(bufferData, data, width*height*nrOfComponents, type);
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
    unsigned int nrOfVoxels = width*height*depth;
    if(nrOfComponents == 3) {
        nrOfVoxels *= 4;
    } else {
        nrOfVoxels *= nrOfComponents;
    }
    switch(type) {
        fastSwitchTypeMacro(
            bufferData = new FAST_TYPE[nrOfVoxels];
        );
    }
    device->getCommandQueue().enqueueReadImage(image, CL_TRUE, oul::createOrigoRegion(), oul::createRegion(width, height, depth), 0, 0, bufferData);

    if(nrOfComponents == 3) {
        // Since OpenCL images doesn't support 3 channels, 4 channel image is used and we must remove the padding
        switch(type) {
            fastSwitchTypeMacro(
                void * newbufferData = removePadding((FAST_TYPE*)bufferData, width*height*depth);
                deleteArray(bufferData, type);
                bufferData = newbufferData;
            );
        }
    }

    return compareDataArrays(bufferData, data, width*height*depth*nrOfComponents, type);
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

TEST_CASE("Create a 2D image and change host data", "[fast][image]") {
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
            deleteArray(data, type);

            // Put the data as buffer and host data as well
            ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE);
            void* changedData = access2.get();

            // Now change host data
            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)changedData;
                    for(int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }
            access2.release();

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, changedData, width*height*nrOfComponents, type) == true);
            access.release();

            OpenCLImageAccess2D access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access3.get();
            CHECK(compareImage2DWithDataArray(*clImage, device, changedData, width, height, nrOfComponents, type) == true);

        }
    }
}

TEST_CASE("Create a 3D image and change host data", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 40;
    unsigned int height = 40;
    unsigned int depth = 40;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);
            deleteArray(data, type);

            // Put the data as buffer and host data as well
            ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE);
            void* changedData = access2.get();

            // Now change host data
            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)changedData;
                    for(int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }
            access2.release();

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, changedData, width*height*depth*nrOfComponents, type) == true);
            access.release();

            OpenCLImageAccess3D access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access3.get();
            CHECK(compareImage3DWithDataArray(*clImage, device, changedData, width, height, depth, nrOfComponents, type) == true);

        }
    }
}

TEST_CASE("Create a 2D image and change buffer data", "[fast][image]") {
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

            // Change buffer data
            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Buffer* buffer = access.get();

            // Create kernel for changing buffer data
            std::string typeDef;
            switch(type) {
            case TYPE_FLOAT:
                typeDef = "-D FAST_TYPE=float";
                break;
            case TYPE_INT8:
                typeDef = "-D FAST_TYPE=char";
                break;
            case TYPE_UINT8:
                typeDef = "-D FAST_TYPE=uchar";
                break;
            case TYPE_INT16:
                typeDef = "-D FAST_TYPE=short";
                break;
            case TYPE_UINT16:
                typeDef = "-D FAST_TYPE=ushort";
                break;
            }
            int i = device->createProgramFromString("__kernel void changeData(__global FAST_TYPE* buffer) {"
                    "buffer[get_global_id(0)] = buffer[get_global_id(0)]*2; "
                    "}", typeDef);
            cl::Kernel kernel(device->getProgram(i), "changeData");
            kernel.setArg(0, *buffer);
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width*height*nrOfComponents),
                    cl::NullRange
            );
            access.release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }

            ImageAccess access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2.get(), data, width*height*nrOfComponents, type) == true);

            OpenCLImageAccess2D access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access3.get();
            CHECK(compareImage2DWithDataArray(*clImage, device, data, width, height, nrOfComponents, type) == true);
            deleteArray(data, type);

        }
    }
}

TEST_CASE("Create a 3D image and change buffer data", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    unsigned int width = 32;
    unsigned int height = 64;
    unsigned int depth = 32;


    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);


            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);

            // Change buffer data
            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Buffer* buffer = access.get();

            // Create kernel for changing buffer data
            std::string typeDef;
            switch(type) {
            case TYPE_FLOAT:
                typeDef = "-D FAST_TYPE=float";
                break;
            case TYPE_INT8:
                typeDef = "-D FAST_TYPE=char";
                break;
            case TYPE_UINT8:
                typeDef = "-D FAST_TYPE=uchar";
                break;
            case TYPE_INT16:
                typeDef = "-D FAST_TYPE=short";
                break;
            case TYPE_UINT16:
                typeDef = "-D FAST_TYPE=ushort";
                break;
            }
            int i = device->createProgramFromString("__kernel void changeData(__global FAST_TYPE* buffer) {"
                    "buffer[get_global_id(0)] = buffer[get_global_id(0)]*2; "
                    "}", typeDef);
            cl::Kernel kernel(device->getProgram(i), "changeData");
            kernel.setArg(0, *buffer);
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width*height*depth*nrOfComponents),
                    cl::NullRange
            );
            access.release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }

            ImageAccess access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2.get(), data, width*height*depth*nrOfComponents, type) == true);

            OpenCLImageAccess3D access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access3.get();
            CHECK(compareImage3DWithDataArray(*clImage, device, data, width, height, depth, nrOfComponents, type) == true);
            deleteArray(data, type);

        }
    }
}

TEST_CASE("Create a 2D image and change image data", "[fast][image]") {
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

            // Change image data
            OpenCLImageAccess2D access3 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            cl::Image2D* clImage = access3.get();

            // Create kernel for changing image data
            int i;
            if(type == TYPE_FLOAT) {
                i = device->createProgramFromString("__kernel void changeData(__write_only image2d_t image) {"
                        "int2 pos = {get_global_id(0), get_global_id(1)};"
                        "write_imagef(image, pos, (float4)(1,1,1,1));"
                        "}");
            } else {
                i = device->createProgramFromString("__kernel void changeData(__write_only image2d_t image) {"
                        "int2 pos = {get_global_id(0), get_global_id(1)};"
                        "write_imagei(image, pos, (int4)(1,1,1,1));"
                        "}");
            }
            cl::Kernel kernel(device->getProgram(i), "changeData");
            kernel.setArg(0, *clImage);
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
            access3.release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = 1;
                    }
                )
            }

            ImageAccess access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2.get(), data, width*height*nrOfComponents, type) == true);

            OpenCLBufferAccess access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access.get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*nrOfComponents, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create a 3D image and change image data", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    // check if device has 3D write capabilities, if not abort test
    if(device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") == std::string::npos) {
        return;
    }

    unsigned int width = 32;
    unsigned int height = 32;
    unsigned int depth = 32;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);

            // Change image data
            OpenCLImageAccess3D access3 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
            cl::Image3D* clImage = access3.get();

            // Create kernel for changing image data
            int i;
            if(type == TYPE_FLOAT) {
                i = device->createProgramFromString("__kernel void changeData(__write_only image3d_t image) {"
                        "int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};"
                        "write_imagef(image, pos, (float4)(1,1,1,1));"
                        "}");
            } else {
                i = device->createProgramFromString("__kernel void changeData(__write_only image3d_t image) {"
                        "int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};"
                        "write_imagei(image, pos, (int4)(1,1,1,1));"
                        "}");
            }
            cl::Kernel kernel(device->getProgram(i), "changeData");
            kernel.setArg(0, *clImage);
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height, depth),
                    cl::NullRange
            );
            access3.release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = 1;
                    }
                )
            }

            ImageAccess access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2.get(), data, width*height*depth*nrOfComponents, type) == true);

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
