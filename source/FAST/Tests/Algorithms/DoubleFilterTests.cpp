#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "DoubleFilter.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

TEST_CASE("DoubleFilter on OpenCL device", "[fast][DoubleFilter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Heart/ApicalFourChamber/US-2D_0.mhd");

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->update();
    Reporter::info() << "finished update" << Reporter::end();

    Image::pointer input = importer->getOutputData<Image>(0);
    Image::pointer output = filter->getOutputData<Image>(0);
    input->retain(filter->getMainDevice());
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ);

    uchar* inputData = (uchar*)inputAccess->get();
    uchar* outputData = (uchar*)outputAccess->get();
    bool success = true;
    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        uchar result = inputData[i]*2;
        if(result != outputData[i]) {
            success = false;
            break;
        }
    }
    CHECK(success == true);
}

TEST_CASE("DoubleFilter on Host", "[fast][DoubleFilter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Heart/ApicalFourChamber/US-2D_0.mhd");

    DoubleFilter::pointer filter = DoubleFilter::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->setMainDevice(Host::getInstance());
    filter->update();
    Reporter::info() << "finished update" << Reporter::end();

    Image::pointer input = importer->getOutputData<Image>(0);
    Image::pointer output = filter->getOutputData<Image>(0);
    input->retain(filter->getMainDevice());

    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ);

    uchar* inputData = (uchar*)inputAccess->get();
    uchar* outputData = (uchar*)outputAccess->get();
    bool success = true;
    for(unsigned int i = 0; i < input->getWidth()*input->getHeight(); i++) {
        uchar result = inputData[i]*2;
        if(result != outputData[i]) {
            success = false;
            break;
        }
    }
    CHECK(success == true);
}
