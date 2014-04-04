#include "catch.hpp"
#include "ITKImageExporter.hpp"
#include "ImageImporter2D.hpp"

using namespace fast;

TEST_CASE("Export image to ITK from FAST", "[fast][ITK]") {

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    Image2D::pointer fastImage = importer->getOutput();

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
