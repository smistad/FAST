#include "FAST/Tests/catch.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Tests/DataComparison.hpp"
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
    image->create2DImage(width, height, type, nrOfComponents, Host::getInstance());

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
    image->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance());

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
            ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access->get(), width*height*nrOfComponents, type) == true);
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
            ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access->get(), width*height*depth*nrOfComponents, type) == true);
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
            image->create2DImage(width, height, type, nrOfComponents, Host::getInstance(), data);

            ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access->get(), width*height*nrOfComponents, type) == true);

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
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance(), data);

            ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(data, access->get(), width*height*depth*nrOfComponents, type) == true);

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
            image->create2DImage(width, height, type, nrOfComponents, Host::getInstance(), data);

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
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
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance(), data);

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
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
            image->create2DImage(width, height, type, nrOfComponents, Host::getInstance(), data);

            OpenCLImageAccess2D::pointer access = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access->get();
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
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance(), data);

            OpenCLImageAccess3D::pointer access = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access->get();
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

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
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

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
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
            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE);
            void* changedData = access2->get();

            // Now change host data
            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)changedData;
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }
            access2->release();

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
            CHECK(compareBufferWithDataArray(*buffer, device, changedData, width*height*nrOfComponents, type) == true);
            access->release();

            OpenCLImageAccess2D::pointer access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access3->get();
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
            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE);
            void* changedData = access2->get();

            // Now change host data
            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)changedData;
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }
            access2->release();

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
            CHECK(compareBufferWithDataArray(*buffer, device, changedData, width*height*depth*nrOfComponents, type) == true);
            access->release();

            OpenCLImageAccess3D::pointer access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access3->get();
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
            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Buffer* buffer = access->get();

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
            access->release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }

            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2->get(), data, width*height*nrOfComponents, type) == true);

            OpenCLImageAccess2D::pointer access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
            cl::Image2D* clImage = access3->get();
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
            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            cl::Buffer* buffer = access->get();

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
            access->release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = changedData2[i]*2;
                    }
                )
            }

            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2->get(), data, width*height*depth*nrOfComponents, type) == true);

            OpenCLImageAccess3D::pointer access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            cl::Image3D* clImage = access3->get();
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
            OpenCLImageAccess2D::pointer access3 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            cl::Image2D* clImage = access3->get();

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
            access3->release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(unsigned int i = 0; i < width*height*nrOfComponents; i++) {
                        changedData2[i] = 1;
                    }
                )
            }

            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2->get(), data, width*height*nrOfComponents, type) == true);

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
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
            INFO("Components " << nrOfComponents);
            INFO("Type " << typeNr);
            DataType type = (DataType)typeNr;

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth*nrOfComponents, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, nrOfComponents, device, data);

            // Change image data
            OpenCLImageAccess3D::pointer access3 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
            cl::Image3D* clImage = access3->get();

            // Create kernel for changing image data
            int i;
            if(type == TYPE_FLOAT) {
                i = device->createProgramFromString("__kernel void changeData(__write_only image3d_t image) {"
                        "int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};"
                        "write_imagef(image, pos, (float4)(1,1,1,1));"
                        "}");
            } else if(type == TYPE_UINT8 || type == TYPE_UINT16) {
                 i = device->createProgramFromString("__kernel void changeData(__write_only image3d_t image) {"
                        "int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};"
                        "write_imageui(image, pos, (uint4)(1,1,1,1));"
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
            access3->release();

            switch(type) {
                fastSwitchTypeMacro(
                    FAST_TYPE* changedData2 = (FAST_TYPE*)data;
                    for(unsigned int i = 0; i < width*height*depth*nrOfComponents; i++) {
                        changedData2[i] = 1;
                    }
                )
            }

            ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ);
            CHECK(compareDataArrays(access2->get(), data, width*height*depth*nrOfComponents, type) == true);

            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, device);
            cl::Buffer* buffer = access->get();
            CHECK(compareBufferWithDataArray(*buffer, device, data, width*height*depth*nrOfComponents, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create 2D image on one OpenCL device and request it on another OpenCL device", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    std::vector<OpenCLDevice::pointer> devices = deviceManager.getAllDevices();

    // If system only has 1 device, skip this test
    if(devices.size() <= 1)
        return;

    unsigned int width = 32;
    unsigned int height = 32;
    DataType type = TYPE_FLOAT;

    // Try to transfer data between all OpenCL devices
    for(int from = 0; from < devices.size(); from++) {
        for(int to = 0; to < devices.size(); to++) {
            if(to == from)
                continue;
            // Create image on "from" device
            INFO("From: " << devices[from]->getName());
            INFO("To: " << devices[to]->getName());

            // Create a data array with random data
            void* data = allocateRandomData(width*height, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, 1, devices[from], data);

            // Request image on "to" device
            OpenCLImageAccess2D::pointer access = image->getOpenCLImageAccess2D(ACCESS_READ, devices[to]);
            cl::Image2D* clImage = access->get();
            CHECK(compareImage2DWithDataArray(*clImage, devices[to], data, width, height, 1, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create 3D image on one OpenCL device and request it on another OpenCL device", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    std::vector<OpenCLDevice::pointer> devices = deviceManager.getAllDevices();

    // If system only has 1 device, skip this test
    if(devices.size() <= 1)
        return;

    unsigned int width = 32;
    unsigned int height = 32;
    unsigned int depth = 32;
    DataType type = TYPE_FLOAT;

    // Try to transfer data between all OpenCL devices
    for(int from = 0; from < devices.size(); from++) {
        for(int to = 0; to < devices.size(); to++) {
            if(to == from)
                continue;
            // Create image on "from" device
            INFO("From: " << devices[from]->getName());
            INFO("To: " << devices[to]->getName());

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, 1, devices[from], data);

            // Request image on "to" device
            OpenCLImageAccess3D::pointer access = image->getOpenCLImageAccess3D(ACCESS_READ, devices[to]);
            cl::Image3D* clImage = access->get();
            CHECK(compareImage3DWithDataArray(*clImage, devices[to], data, width, height, depth, 1, type) == true);
            deleteArray(data, type);
        }
    }
}


TEST_CASE("Create 2D image on one OpenCL device and request it on another OpenCL device as buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    std::vector<OpenCLDevice::pointer> devices = deviceManager.getAllDevices();

    // If system only has 1 device, skip this test
    if(devices.size() <= 1)
        return;

    unsigned int width = 32;
    unsigned int height = 32;
    DataType type = TYPE_FLOAT;

    // Try to transfer data between all OpenCL devices
    for(int from = 0; from < devices.size(); from++) {
        for(int to = 0; to < devices.size(); to++) {
            if(to == from)
                continue;
            // Create image on "from" device
            INFO("From: " << devices[from]->getName());
            INFO("To: " << devices[to]->getName());

            // Create a data array with random data
            void* data = allocateRandomData(width*height, type);

            Image::pointer image = Image::New();
            image->create2DImage(width, height, type, 1, devices[from], data);

            // Request image on "to" device
            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, devices[to]);
            cl::Buffer* buffer = access->get();
            CHECK(compareBufferWithDataArray(*buffer, devices[to], data, width*height*1, type) == true);
            deleteArray(data, type);
        }
    }
}

TEST_CASE("Create 3D image on one OpenCL device and request it on another OpenCL device as buffer", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    std::vector<OpenCLDevice::pointer> devices = deviceManager.getAllDevices();

    // If system only has 1 device, skip this test
    if(devices.size() <= 1)
        return;

    unsigned int width = 32;
    unsigned int height = 32;
    unsigned int depth = 32;
    DataType type = TYPE_FLOAT;

    // Try to transfer data between all OpenCL devices
    for(int from = 0; from < devices.size(); from++) {
        for(int to = 0; to < devices.size(); to++) {
            if(to == from)
                continue;
            // Create image on "from" device
            INFO("From: " << devices[from]->getName());
            INFO("To: " << devices[to]->getName());

            // Create a data array with random data
            void* data = allocateRandomData(width*height*depth, type);

            Image::pointer image = Image::New();
            image->create3DImage(width, height, depth, type, 1, devices[from], data);

            // Request image on "to" device
            OpenCLBufferAccess::pointer access = image->getOpenCLBufferAccess(ACCESS_READ, devices[to]);
            cl::Buffer* clImage = access->get();
            CHECK(compareBufferWithDataArray(*clImage, devices[to], data, width*height*depth*1, type) == true);
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
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());

    CHECK_NOTHROW(ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ));
    CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
    CHECK_NOTHROW(OpenCLImageAccess2D::pointer access3 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
    CHECK_NOTHROW(ImageAccess::pointer access4 = image->getImageAccess(ACCESS_READ));
}

TEST_CASE("Multiple read access to 3D image should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(64, 64, 32, TYPE_FLOAT, 1, Host::getInstance());

    CHECK_NOTHROW(
            ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ);
            OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device);
            OpenCLImageAccess3D::pointer access3 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
            ImageAccess::pointer access4 = image->getImageAccess(ACCESS_READ)
    );
}

TEST_CASE("Requesting access to a 2D image that is being written to should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ_WRITE);
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));		
    }

    {
        OpenCLImageAccess2D::pointer access1 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting access to a 3D image that is being written to should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ_WRITE);
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D::pointer access1 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}





TEST_CASE("Requesting access to a 2D image that has been released should not throw exception", "[fast][image]") {

	

    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ_WRITE);
        access1->release();
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D::pointer access1 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting access to a 3D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ_WRITE);
        access1->release();
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D::pointer access1 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ));
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ, device));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 2D image that is being read from should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ);
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D::pointer access1 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 3D image that is being read from should throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ);
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D::pointer access1 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
        CHECK_THROWS(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_THROWS(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 2D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ);
        access1->release();
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess2D::pointer access2 = image->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess2D::pointer access1 = image->getOpenCLImageAccess2D(ACCESS_READ, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Requesting write access to a 3D image that has been released should not throw exception", "[fast][image]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    Image::pointer image = Image::New();
    image->create3DImage(32,32,32, TYPE_FLOAT, 1, Host::getInstance());

    {
        ImageAccess::pointer access1 = image->getImageAccess(ACCESS_READ);
        access1->release();
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLBufferAccess::pointer access1 = image->getOpenCLBufferAccess(ACCESS_READ, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLImageAccess3D::pointer access2 = image->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device));
    }

    {
        OpenCLImageAccess3D::pointer access1 = image->getOpenCLImageAccess3D(ACCESS_READ, device);
        access1->release();
        CHECK_NOTHROW(ImageAccess::pointer access2 = image->getImageAccess(ACCESS_READ_WRITE));
        CHECK_NOTHROW(OpenCLBufferAccess::pointer access2 = image->getOpenCLBufferAccess(ACCESS_READ_WRITE, device));
    }
}

TEST_CASE("Initialize an image twice does not throw exception", "[fast][image]") {
    Image::pointer image = Image::New();

    image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance());
    CHECK_NOTHROW(image->create2DImage(256, 256, TYPE_FLOAT, 1, Host::getInstance()));
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
            image->create2DImage(width, height, type, nrOfComponents, Host::getInstance(), data);

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
            image->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance(), data);

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
    image1->create2DImage(width, height, type, nrOfComponents, Host::getInstance());

    // Create some metadata
    Vector3f spacing;
    spacing[0] = 1.2;
    spacing[1] = 2.3;
    spacing[2] = 1;

    image1->setSpacing(spacing);

    image2->createFromImage(image1, Host::getInstance());

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == 1);
    CHECK(image2->getDimensions() == 2);
    CHECK(image2->getNrOfComponents() == nrOfComponents);
    CHECK(image2->getDataType() == type);

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
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
    image1->create3DImage(width, height, depth, type, nrOfComponents, Host::getInstance());

    // Create some metadata
    Vector3f spacing;
    spacing[0] = 1.2;
    spacing[1] = 2.3;
    spacing[2] = 1;

    image1->setSpacing(spacing);

    image2->createFromImage(image1, Host::getInstance());

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == depth);
    CHECK(image2->getDimensions() == 3);
    CHECK(image2->getNrOfComponents() == nrOfComponents);
    CHECK(image2->getDataType() == type);

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
    }
}


