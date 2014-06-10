#include "catch.hpp"
#include "Image.hpp"
#include "DeviceManager.hpp"
#include "DataComparison.hpp"
#include <limits>

using namespace fast;

// Undefine windows crap
#undef min
#undef max

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
    CHECK(image->getSpacing().x() == Approx(1));
    CHECK(image->getSpacing().y() == Approx(1));
    CHECK(image->getSpacing().z() == Approx(1));
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
    CHECK(image->getSpacing().x() == Approx(1));
    CHECK(image->getSpacing().y() == Approx(1));
    CHECK(image->getSpacing().z() == Approx(1));
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
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
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
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
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
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
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
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
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
            } else if(type == TYPE_INT8 || type == TYPE_INT16) {
                i = device->createProgramFromString("__kernel void changeData(__write_only image2d_t image) {"
                        "int2 pos = {get_global_id(0), get_global_id(1)};"
                        "write_imagei(image, pos, (int4)(1,1,1,1));"
                        "}");
            } else {
				i = device->createProgramFromString("__kernel void changeData(__write_only image2d_t image) {"
                        "int2 pos = {get_global_id(0), get_global_id(1)};"
                        "write_imageui(image, pos, (uint4)(1,1,1,1));"
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
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
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
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
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

TEST_CASE("Uninitialized image throws exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();

    CHECK_THROWS(image->getImageAccess(ACCESS_READ));
    CHECK_THROWS(image->getOpenCLBufferAccess(ACCESS_READ, device));
    CHECK_THROWS(image->getOpenCLImageAccess2D(ACCESS_READ, device));
    CHECK_THROWS(image->getOpenCLImageAccess3D(ACCESS_READ, device));

    CHECK_THROWS(image->getWidth());
    CHECK_THROWS(image->getHeight());
    CHECK_THROWS(image->getDepth());
    CHECK_THROWS(image->getDimensions());
    CHECK_THROWS(image->getNrOfComponents());
    CHECK_THROWS(image->getDataType());
}

TEST_CASE("Multiple read access to 2D image should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());

    CHECK_NOTHROW(ImageAccess access1 = image->getImageAccess(ACCESS_READ));
    CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
    CHECK_NOTHROW(OpenCLImageAccess2D access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
    CHECK_NOTHROW(ImageAccess access4 = image->getImageAccess(ACCESS_READ));
}

TEST_CASE("Multiple read access to 3D image should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(64, 64, 32, TYPE_FLOAT, 1, Host::New());

    CHECK_NOTHROW(
            ImageAccess access1 = image->getImageAccess(ACCESS_READ);
            OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device);
            OpenCLImageAccess3D access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            ImageAccess access4 = image->getImageAccess(ACCESS_READ)
    );
}

TEST_CASE("Requesting access to a 2D image that is being written to should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ_WRITE);
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D access1 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting access to a 3D image that is being written to should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ_WRITE);
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D access1 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting access to a 2D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ_WRITE);
        access1.release();
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D access1 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting access to a 3D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ_WRITE);
        access1.release();
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D access1 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 2D image that is being read from should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ);
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D access1 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 3D image that is being read from should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ);
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D access1 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 2D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ);
        access1.release();
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess2D access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D access1 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 3D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::New());

    {
        ImageAccess access1 = image->getImageAccess(ACCESS_READ);
        access1.release();
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess3D access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D access1 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
        access1.release();
        CHECK_NOTHROW(ImageAccess access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Initialize an image twice throws exception", "[fast][image]") {
    Image::pointer image = Image::New();

    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New());
    CHECK_THROWS(image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::New()));
}

TEST_CASE("Calling calculate max or min intensity on uninitialized image throws an exception", "[fast][image]") {
    Image::pointer image = Image::New();
    CHECK_THROWS(image->calculateMaximumIntensity());
    CHECK_THROWS(image->calculateMinimumIntensity());
}

template <class T>
inline void getMaxAndMinFromData(void* voidData, unsigned int nrOfElements, float* min, float* max) {
    T* data = (T*)voidData;

    *min = std::numeric_limits<float>::max();
    *max = std::numeric_limits<float>::min();
    for(unsigned int i = 0; i < nrOfElements; i++) {
        if((float)data[i] < *min) {
            *min = (float)data[i];
        }
        if((float)data[i] > *max) {
            *max = (float)data[i];
        }
    }
}

inline void getMaxAndMinFromData(void* data, unsigned int nrOfElements, float* min, float* max, DataType type) {
    switch(type) {
    case TYPE_FLOAT:
        getMaxAndMinFromData<float>(data,nrOfElements,min,max);
        break;
    case TYPE_INT8:
        getMaxAndMinFromData<char>(data,nrOfElements,min,max);
        break;
    case TYPE_UINT8:
        getMaxAndMinFromData<uchar>(data,nrOfElements,min,max);
        break;
    case TYPE_INT16:
        getMaxAndMinFromData<short>(data,nrOfElements,min,max);
        break;
    case TYPE_UINT16:
        getMaxAndMinFromData<ushort>(data,nrOfElements,min,max);
        break;
    }
}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 2D image stored as OpenCL image" , "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    unsigned int width = 31;
    unsigned int height = 64;

    // Test for having components 1 to 4 and for all data types
    unsigned int nrOfComponents = 1;
    //for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, device, data);

            float min,max;
            getMaxAndMinFromData(data, width*height*nrOfComponents, &min, &max, type);
            CHECK(image->calculateMaximumIntensity() == Approx(max));
            CHECK(image->calculateMinimumIntensity() == Approx(min));
        }
    //}
}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 3D image stored as OpenCL image" , "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    unsigned int width = 32;
    unsigned int height = 43;
    unsigned int depth = 11;

    // Test for having components 1 to 4 and for all data types
    unsigned int nrOfComponents = 1;
    //for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);

            float min,max;
            getMaxAndMinFromData(data, width*height*depth*nrOfComponents, &min, &max, type);
            CHECK(image->calculateMaximumIntensity() == Approx(max));
            CHECK(image->calculateMinimumIntensity() == Approx(min));
        }
    //}
}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 2D image stored as OpenCL buffer" , "[fast][image]") {

}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 3D image stored as OpenCL buffer" , "[fast][image]") {

}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 2D image stored as host array" , "[fast][image]") {
    unsigned int width = 32;
    unsigned int height = 32;

    // Test for having components 1 to 4 and for all data types
    for(unsigned int nrOfComponents = 1; nrOfComponents <= 4; nrOfComponents++) {
        for(unsigned int typeNr = 0; typeNr < 5; typeNr++) {
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, nrOfComponents, Host::New(), data);

            float min,max;
            getMaxAndMinFromData(data, width*height*nrOfComponents, &min, &max, type);
            CHECK(image->calculateMaximumIntensity() == Approx(max));
            CHECK(image->calculateMinimumIntensity() == Approx(min));
        }
    }
}

TEST_CASE("calculateMaximum/MinimumIntensity returns the maximum/minimum intensity of a 3D image stored as host array" , "[fast][image]") {
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
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::New(), data);

            float min,max;
            getMaxAndMinFromData(data, width*height*depth*nrOfComponents, &min, &max, type);
            CHECK(image->calculateMaximumIntensity() == Approx(max));
            CHECK(image->calculateMinimumIntensity() == Approx(min));
        }
    }
}

TEST_CASE("createFromImage on 2D image", "[fast][image]") {

    Image::pointer image1 = Image::New();
    Image::pointer image2 = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int nrOfComponents = 2;
    DataType type = TYPE_INT8;
    image1->create2DImage(width, height, type, nrOfComponents, Host::New());

    // Create some metadata
    Float<3> spacing;
    spacing[0] = 1.2;
    spacing[1] = 2.3;
    spacing[2] = 1;
    Float<3> offset;
    offset[0] = 2.2;
    offset[1] = 3.3;
    offset[2] = 3.1;
    Float<3> centerOfRotation;
    centerOfRotation[0] = 3.2;
    centerOfRotation[1] = 4.3;
    centerOfRotation[2] = 5.0;
    Float<9> transformMatrix;
    transformMatrix[0] = 0.2;
    transformMatrix[1] = 1.3;
    transformMatrix[2] = 2.0;
    transformMatrix[3] = 3.0;
    transformMatrix[4] = 4.0;
    transformMatrix[5] = 5.0;
    transformMatrix[6] = 6.0;
    transformMatrix[7] = 7.0;
    transformMatrix[8] = 8.0;

    image1->setSpacing(spacing);
    image1->setOffset(offset);
    image1->setCenterOfRotation(centerOfRotation);
    image1->setTransformMatrix(transformMatrix);

    image2->createFromImage(image1, Host::New());

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == 1);
    CHECK(image2->getDimensions() == 2);
    CHECK(image2->getNrOfComponents() == nrOfComponents);
    CHECK(image2->getDataType() == type);

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
        CHECK(offset[i] == Approx(image2->getOffset()[i]));
        CHECK(centerOfRotation[i] == Approx(image2->getCenterOfRotation()[i]));
    }
    for(unsigned int i = 0; i < 9; i++) {
        CHECK(transformMatrix[i] == Approx(image2->getTransformMatrix()[i]));
    }
}

TEST_CASE("createFromImage on 3D image", "[fast][image]") {

    Image::pointer image1 = Image::New();
    Image::pointer image2 = Image::New();

    unsigned int width = 256;
    unsigned int height = 512;
    unsigned int depth = 45;
    unsigned int nrOfComponents = 2;
    DataType type = TYPE_INT8;
    image1->create3DImage(width, height, depth, type, nrOfComponents, Host::New());

    // Create some metadata
    Float<3> spacing;
    spacing[0] = 1.2;
    spacing[1] = 2.3;
    spacing[2] = 1;
    Float<3> offset;
    offset[0] = 2.2;
    offset[1] = 3.3;
    offset[2] = 3.1;
    Float<3> centerOfRotation;
    centerOfRotation[0] = 3.2;
    centerOfRotation[1] = 4.3;
    centerOfRotation[2] = 5.0;
    Float<9> transformMatrix;
    transformMatrix[0] = 0.2;
    transformMatrix[1] = 1.3;
    transformMatrix[2] = 2.0;
    transformMatrix[3] = 3.0;
    transformMatrix[4] = 4.0;
    transformMatrix[5] = 5.0;
    transformMatrix[6] = 6.0;
    transformMatrix[7] = 7.0;
    transformMatrix[8] = 8.0;

    image1->setSpacing(spacing);
    image1->setOffset(offset);
    image1->setCenterOfRotation(centerOfRotation);
    image1->setTransformMatrix(transformMatrix);

    image2->createFromImage(image1, Host::New());

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == depth);
    CHECK(image2->getDimensions() == 3);
    CHECK(image2->getNrOfComponents() == nrOfComponents);
    CHECK(image2->getDataType() == type);

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
        CHECK(offset[i] == Approx(image2->getOffset()[i]));
        CHECK(centerOfRotation[i] == Approx(image2->getCenterOfRotation()[i]));
    }
    for(unsigned int i = 0; i < 9; i++) {
        CHECK(transformMatrix[i] == Approx(image2->getTransformMatrix()[i]));
    }
}


