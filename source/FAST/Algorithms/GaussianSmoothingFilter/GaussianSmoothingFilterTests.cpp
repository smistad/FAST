#include "FAST/Tests/catch.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

TEST_CASE("No input given to GaussianSmoothingFilter throws exception", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    CHECK_THROWS(filter->update());
}

TEST_CASE("Negative or zero sigma and mask size input throws exception in GaussianSmoothingFilter" , "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();

    CHECK_THROWS(filter->setMaskSize(-4));
    CHECK_THROWS(filter->setMaskSize(0));
    CHECK_THROWS(filter->setStandardDeviation(-4));
    CHECK_THROWS(filter->setStandardDeviation(0));
}

TEST_CASE("Even input as mask size throws exception in GaussianSmoothingFilter", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();

    CHECK_THROWS(filter->setMaskSize(2));
}

TEST_CASE("Correct output with small 3x3 2D image as input to GaussianSmoothingFilter on OpenCLDevice", "[fast][GaussianSmoothingFilter]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setMaskSize(3);
    filter->setStandardDeviation(1.0);

    Image::pointer image = Image::New();
    image->create2DImage(3,3,TYPE_FLOAT,1,device);
    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ_WRITE);
    float* data = (float*)access->get();
    for(unsigned int i = 0; i < 9; i++) {
        data[i] = 1.0f;
    }
    access->release();
    filter->setInputData(image);
    Image::pointer output = filter->getOutputData<Image>();
    filter->update();

    ImageAccess::pointer access2 = output->getImageAccess(ACCESS_READ);
    data = (float*)access2->get();

    bool success = true;
    for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
        float truth;
        unsigned int distance = abs(x-1)+abs(y-1);
        if(distance == 2) {
            truth = 0.526976f;
        } else if(distance == 1) {
            truth = 0.725931f;
        } else {
            truth = 1.0f;
        }
        if(fabs(data[x+y*3] - truth) > 0.00001) {
            success = false;
            break;
        }
    }}

    CHECK(success == true);
}

// TODO fix this test
TEST_CASE("Correct output with small 3x3 2D image as input to GaussianSmoothingFilter on Host", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setMaskSize(3);
    filter->setStandardDeviation(1.0);
    //filter->setDevice(Host::New());

    Image::pointer image = Image::New();
    image->create2DImage(3,3,TYPE_FLOAT,1,Host::getInstance());
    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ_WRITE);
    float* data = (float*)access->get();
    for(unsigned int i = 0; i < 9; i++) {
        data[i] = 1.0f;
    }
    access->release();
    filter->setInputData(image);
    Image::pointer output = filter->getOutputData<Image>();
    filter->update();

	ImageAccess::pointer access2 = output->getImageAccess(ACCESS_READ);
	data = (float*)access2->get();

    bool success = true;
    for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
        float truth;
        unsigned int distance = abs(x-1)+abs(y-1);
        if(distance == 2) {
            truth = 0.526976f;
        } else if(distance == 1) {
            truth = 0.725931f;
        } else {
            truth = 1.0f;
        }
        if(fabs(data[x+y*3] - truth) > 0.00001) {
            success = false;
            break;
        }
    }}

    CHECK(success == true);
}

TEST_CASE("Correct output with small 3x3x3 3D image as input to GaussianSmoothingFilter on OpenCLDevice", "[fast][GaussianSmoothingFilter]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setMaskSize(3);
    filter->setStandardDeviation(1.0);

    Image::pointer image = Image::New();
    image->create3DImage(3,3,3,TYPE_FLOAT,1,device);
    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ_WRITE);
    float* data = (float*)access->get();
    for(unsigned int i = 0; i < 3*3*3; i++) {
        data[i] = 1.0f;
    }
    access->release();
    filter->setInputData(image);
    Image::pointer output = filter->getOutputData<Image>();
    filter->update();

	ImageAccess::pointer access2 = output->getImageAccess(ACCESS_READ);
	data = (float*)access2->get();

    bool success = true;
    for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
    for(int z = 0; z < 3; z++) {
        float truth;
        unsigned int distance = abs(x-1)+abs(y-1)+abs(z-1);
        if(distance == 3) {
            truth = 0.382549;
        } else if(distance == 2) {
            truth = 0.526976f;
        } else if(distance == 1) {
            truth = 0.725931f;
        } else {
            truth = 1.0f;
        }
        if(fabs(data[x+y*3+z*3*3] - truth) > 0.00001) {
            success = false;
            break;
        }
    }}}

    CHECK(success == true);
}

TEST_CASE("Correct output with small 3x3x3 3D image as input to GaussianSmoothingFilter on Host", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setMaskSize(3);
    filter->setStandardDeviation(1.0);

    Image::pointer image = Image::New();
    image->create3DImage(3,3,3,TYPE_FLOAT,1,Host::getInstance());
    ImageAccess::pointer access = image->getImageAccess(ACCESS_READ_WRITE);
    float* data = (float*)access->get();
    for(unsigned int i = 0; i < 3*3*3; i++) {
        data[i] = 1.0f;
    }
    access->release();
    filter->setInputData(image);
    Image::pointer output = filter->getOutputData<Image>();
    filter->update();

	ImageAccess::pointer access2 = output->getImageAccess(ACCESS_READ);
	data = (float*)access2->get();

    bool success = true;
    for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
    for(int z = 0; z < 3; z++) {
        float truth;
        unsigned int distance = abs(x-1)+abs(y-1)+abs(z-1);
        if(distance == 3) {
            truth = 0.382549;
        } else if(distance == 2) {
            truth = 0.526976f;
        } else if(distance == 1) {
            truth = 0.725931f;
        } else {
            truth = 1.0f;
        }
        if(fabs(data[x+y*3+z*3*3] - truth) > 0.00001) {
            success = false;
            break;
        }
    }}}

    CHECK(success == true);
}

} // end namespace fast
