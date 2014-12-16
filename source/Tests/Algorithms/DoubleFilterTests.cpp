#include "catch.hpp"
#include "ImageImporter.hpp"
#include "DoubleFilter.hpp"

using namespace fast;

TEST_CASE("DoubleFilter on OpenCL device", "[fast][DoubleFilter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
    Image::pointer input = importer->getOutput();

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInput(input);
    Image::pointer output = filter->getOutput();
    input->retain(filter->getMainDevice());
    filter->update();

    ImageAccess inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess outputAccess = output->getImageAccess(ACCESS_READ);

    float* inputData = (float*)inputAccess.get();
    float* outputData = (float*)outputAccess.get();
    bool success = true;
    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        if(fabs(inputData[i]*2-outputData[i]) > 0.001) {
            success = false;
            break;
        }
    }
    CHECK(success == true);
}

TEST_CASE("DoubleFilter on Host", "[fast][DoubleFilter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
    Image::pointer input = importer->getOutput();

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInput(input);
    filter->setMainDevice(Host::getInstance());
    input->retain(filter->getMainDevice());
    Image::pointer output = filter->getOutput();
    filter->update();

    ImageAccess inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess outputAccess = output->getImageAccess(ACCESS_READ);

    float* inputData = (float*)inputAccess.get();
    float* outputData = (float*)outputAccess.get();
    bool success = true;
    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        if(fabs(inputData[i]*2-outputData[i]) > 0.001) {
            success = false;
            break;
        }
    }
    CHECK(success == true);
}
