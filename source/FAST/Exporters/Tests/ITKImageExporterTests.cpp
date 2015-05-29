#include "FAST/Tests/catch.hpp"
#include "FAST/Exporters/ITKImageExporter.hpp"
#include "FAST/Tests/DataComparison.hpp"

using namespace fast;

template <class T>
bool compareITKDataWithFASTData(void* itkPixelData, void* fastPixelData, unsigned int nrOfVoxels) {
    for(int i = 0; i < nrOfVoxels; i++) {
        if(((T*)itkPixelData)[i] != ((T*)fastPixelData)[i])
            return false;
    }

    return true;
}

TEST_CASE("No input given to the ITKImageExporter", "[fast][ITK]") {
    typedef itk::Image<float, 2> ImageType;
    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    CHECK_THROWS(itkExporter->Update());
}

TEST_CASE("Different dimensions in input and output image throws exception in the ITKImageExporter", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_FLOAT;
    typedef itk::Image<float, 3> ImageType;

    Image::pointer fastImage = Image::New();
    fastImage->create2DImage(width, height, type, 1, Host::getInstance());
    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    CHECK_THROWS(itkExporter->Update());
}

TEST_CASE("Export a float 2D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_FLOAT;
    typedef itk::Image<float, 2> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height, type);
    fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(itkImage->GetImageDimension() == 2);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height))
    }
    CHECK(success == true);
}

TEST_CASE("Export a char 2D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_INT8;
    typedef itk::Image<char, 2> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height, type);
    fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(itkImage->GetImageDimension() == 2);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height))
    }
    CHECK(success == true);
}

TEST_CASE("Export a uchar 2D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_UINT8;
    typedef itk::Image<unsigned char, 2> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height, type);
    fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(itkImage->GetImageDimension() == 2);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height))
    }
    CHECK(success == true);
}

TEST_CASE("Export a short 2D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_INT16;
    typedef itk::Image<short, 2> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height, type);
    fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(itkImage->GetImageDimension() == 2);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height))
    }
    CHECK(success == true);
}

TEST_CASE("Export a ushort 2D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 40;
    DataType type = TYPE_UINT16;
    typedef itk::Image<unsigned short, 2> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height, type);
    fastImage->create2DImage(width, height, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(itkImage->GetImageDimension() == 2);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height))
    }
    CHECK(success == true);
}

TEST_CASE("Export a float 3D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 20;
    DataType type = TYPE_FLOAT;
    typedef itk::Image<float, 3> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height*depth, type);
    fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getHeight() == region.GetSize()[2]);
    CHECK(itkImage->GetImageDimension() == 3);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height*depth))
    }
    CHECK(success == true);
}

TEST_CASE("Export a char 3D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 20;
    DataType type = TYPE_INT8;
    typedef itk::Image<char, 3> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height*depth, type);
    fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getHeight() == region.GetSize()[2]);
    CHECK(itkImage->GetImageDimension() == 3);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height*depth))
    }
    CHECK(success == true);
}

TEST_CASE("Export a uchar 3D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 20;
    DataType type = TYPE_UINT8;
    typedef itk::Image<unsigned char, 3> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height*depth, type);
    fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getHeight() == region.GetSize()[2]);
    CHECK(itkImage->GetImageDimension() == 3);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height*depth))
    }
    CHECK(success == true);
}

TEST_CASE("Export a short 3D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 20;
    DataType type = TYPE_INT16;
    typedef itk::Image<short, 3> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height*depth, type);
    fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getHeight() == region.GetSize()[2]);
    CHECK(itkImage->GetImageDimension() == 3);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height*depth))
    }
    CHECK(success == true);
}

TEST_CASE("Export a ushort 3D image to ITK from FAST", "[fast][ITK]") {
    unsigned int width = 32;
    unsigned int height = 20;
    unsigned int depth = 20;
    DataType type = TYPE_UINT16;
    typedef itk::Image<unsigned short, 3> ImageType;

    Image::pointer fastImage = Image::New();
    void* data = allocateRandomData(width*height*depth, type);
    fastImage->create3DImage(width, height, depth, type, 1, Host::getInstance(), data);

    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->setInputData(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getHeight() == region.GetSize()[2]);
    CHECK(itkImage->GetImageDimension() == 3);

    void* itkData = itkImage->GetBufferPointer();
    bool success;
    switch(fastImage->getDataType()) {
        fastSwitchTypeMacro(success = compareITKDataWithFASTData<FAST_TYPE>(itkData, data, width*height*depth))
    }
    CHECK(success == true);
}
