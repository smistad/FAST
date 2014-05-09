#include "catch.hpp"
#include "ITKImageExporter.hpp"
#include "ImageImporter.hpp"

using namespace fast;

TEST_CASE("Export image to ITK from FAST", "[fast][ITK]") {

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    Image::pointer fastImage = importer->getOutput();

    // ITK Export example
    typedef itk::Image<float, 2> ImageType;
    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->SetInput(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
}
