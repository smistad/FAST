#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "DoubleFilter.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

TEST_CASE("DoubleFilter on OpenCL device", "[fast][DoubleFilter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->update();
    std::cout << "finished update" << std::endl;

    Image::pointer input = importer->getOutputData<Image>(0);
    Image::pointer output = filter->getOutputData<Image>(0);
    input->retain(filter->getMainDevice());
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ);

    float* inputData = (float*)inputAccess->get();
    float* outputData = (float*)outputAccess->get();
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

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->setMainDevice(Host::getInstance());
    filter->update();
    std::cout << "finished update" << std::endl;

    Image::pointer input = importer->getOutputData<Image>(0);
    Image::pointer output = filter->getOutputData<Image>(0);
    input->retain(filter->getMainDevice());

    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ);

    float* inputData = (float*)inputAccess->get();
    float* outputData = (float*)outputAccess->get();
    bool success = true;
    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        if(fabs(inputData[i]*2-outputData[i]) > 0.001) {
            success = false;
            break;
        }
    }
    CHECK(success == true);
}
