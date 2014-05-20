#include "catch.hpp"
#include "MetaImageExporter.hpp"
#include "MetaImageImporter.hpp"
#include "Image.hpp"
#include "DataComparison.hpp"

using namespace fast;

TEST_CASE("No filename given to the MetaImageExporter", "[fast][MetaImageExporter]") {
    Image::pointer image = Image::New();
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setInput(image);
    CHECK_THROWS(exporter->update());
}

TEST_CASE("No input given to the MetaImageExporter", "[fast][MetaImageExporter]") {
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setFilename("asd");
    CHECK_THROWS(exporter->update());
}

TEST_CASE("Write a 2D image with the MetaImageExporter", "[fast][MetaImageExporter]") {
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
    unsigned int width = 32;
    unsigned int height = 46;
    unsigned int components = 2;
    DataType type = TYPE_UINT8;

    Image::pointer image = Image::New();
    void* data = allocateRandomData(width*height*components, type);
    image->create2DImage(width, height, type, components, Host::New(), data);

    // Set metadata
    image->setSpacing(spacing);
    image->setOffset(offset);
    image->setCenterOfRotation(centerOfRotation);
    image->setTransformMatrix(transformMatrix);

    // Export image
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setFilename("MetaImageExporterTest2D.mhd");
    exporter->setInput(image);
    exporter->update();

    // Import image back again
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("MetaImageExporterTest2D.mhd");
    Image::pointer image2 = importer->getOutput();
    importer->update();

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
        CHECK(offset[i] == Approx(image2->getOffset()[i]));
        CHECK(centerOfRotation[i] == Approx(image2->getCenterOfRotation()[i]));
    }
    for(unsigned int i = 0; i < 9; i++) {
        CHECK(transformMatrix[i] == Approx(image2->getTransformMatrix()[i]));
    }

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == 1);
    CHECK(image2->getDataType() == type);
    CHECK(image2->getNrOfComponents() == components);
    CHECK(image2->getDimensions() == 2);

    ImageAccess access = image2->getImageAccess(ACCESS_READ);
    void* data2 = access.get();
    CHECK(compareDataArrays(data, data2, width*height*components, type) == true);
    deleteArray(data, type);
}


TEST_CASE("Write a 3D image with the MetaImageExporter", "[fast][MetaImageExporter]") {
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
    unsigned int width = 32;
    unsigned int height = 22;
    unsigned int depth = 20;
    unsigned int components = 1;
    DataType type = TYPE_UINT16;

    Image::pointer image = Image::New();
    void* data = allocateRandomData(width*height*depth*components, type);
    image->create3DImage(width, height, depth, type, components, Host::New(), data);

    // Set metadata
    image->setSpacing(spacing);
    image->setOffset(offset);
    image->setCenterOfRotation(centerOfRotation);
    image->setTransformMatrix(transformMatrix);

    // Export image
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    exporter->setFilename("MetaImageExporterTest3D.mhd");
    exporter->setInput(image);
    exporter->update();

    // Import image back again
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("MetaImageExporterTest3D.mhd");
    Image::pointer image2 = importer->getOutput();
    importer->update();

    // Check that the image properties are correct
    for(unsigned int i = 0; i < 3; i++) {
        CHECK(spacing[i] == Approx(image2->getSpacing()[i]));
        CHECK(offset[i] == Approx(image2->getOffset()[i]));
        CHECK(centerOfRotation[i] == Approx(image2->getCenterOfRotation()[i]));
    }
    for(unsigned int i = 0; i < 9; i++) {
        CHECK(transformMatrix[i] == Approx(image2->getTransformMatrix()[i]));
    }

    CHECK(image2->getWidth() == width);
    CHECK(image2->getHeight() == height);
    CHECK(image2->getDepth() == depth);
    CHECK(image2->getDataType() == type);
    CHECK(image2->getNrOfComponents() == components);
    CHECK(image2->getDimensions() == 3);

    ImageAccess access = image2->getImageAccess(ACCESS_READ);
    void* data2 = access.get();
    CHECK(compareDataArrays(data, data2, width*height*depth*components, type) == true);
    deleteArray(data, type);
}
