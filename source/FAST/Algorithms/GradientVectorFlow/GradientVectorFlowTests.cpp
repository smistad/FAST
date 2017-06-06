#include "FAST/Testing.hpp"
#include "EulerGradientVectorFlow.hpp"
#include "MultigridGradientVectorFlow.hpp"
#include "FAST/Algorithms/ScaleImage/ScaleImage.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

Vector2f calculateLaplacian(ImageAccess::pointer& input, ImageAccess::pointer& result, Vector2i position) {
    Vector2f inputVector = input->getVector(position).head(2);
    Vector2f resultVector = result->getVector(position).head(2);
    Vector2f vectorX1 = result->getVector(Vector2i(position + Vector2i(1, 0))).head(2);
    Vector2f vectorY1 = result->getVector(Vector2i(position + Vector2i(0, 1))).head(2);
    Vector2f vectorX_1 = result->getVector(Vector2i(position - Vector2i(1, 0))).head(2);
    Vector2f vectorY_1 = result->getVector(Vector2i(position - Vector2i(0, 1))).head(2);
    return - 4*resultVector + vectorX1 + vectorY1 + vectorX_1 + vectorY_1;
}

Vector3f calculateLaplacian(ImageAccess::pointer& input, ImageAccess::pointer& result, Vector3i position) {
    Vector3f inputVector = input->getVector(position).head(3);
    Vector3f resultVector = result->getVector(position).head(3);
    Vector3f vectorX1 = result->getVector(position + Vector3i(1, 0, 0)).head(3);
    Vector3f vectorY1 = result->getVector(position + Vector3i(0, 1, 0)).head(3);
    Vector3f vectorZ1 = result->getVector(position + Vector3i(0, 0, 1)).head(3);
    Vector3f vectorX_1 = result->getVector(position - Vector3i(1, 0, 0)).head(3);
    Vector3f vectorY_1 = result->getVector(position - Vector3i(0, 1, 0)).head(3);
    Vector3f vectorZ_1 = result->getVector(position - Vector3i(0, 0, 1)).head(3);
    return - 6*resultVector + vectorX1 + vectorY1 + vectorZ1 + vectorX_1 + vectorY_1 + vectorZ_1;
}

/**
 * Calculate average error/residual for each pixel
 */
double calculateGVFVectorFieldResidual(Image::pointer inputVectorField, Image::pointer vectorField, float mu) {
    ImageAccess::pointer input = inputVectorField->getImageAccess(ACCESS_READ);
    ImageAccess::pointer result = vectorField->getImageAccess(ACCESS_READ);
    Reporter::info() << "Calculating residual..." << Reporter::end();

    double sum = 0.0;
    uint size;
    if(inputVectorField->getDimensions() == 2) {
        size = (vectorField->getWidth() - 2)*(vectorField->getHeight() - 2);
        for(uint y = 1; y < vectorField->getHeight()-1; ++y) {
        for(uint x = 1; x < vectorField->getWidth()-1; ++x) {
            Vector2i position(x, y);
            Vector2f inputVector = input->getVector(position).head(2);
            Vector2f resultVector = result->getVector(position).head(2);
            Vector2f laplacian = calculateLaplacian(input, result, position);
            Vector2f residual = mu*laplacian - (inputVector.x()*inputVector.x() + inputVector.y()*inputVector.y())*(resultVector - inputVector);
            sum += residual.norm();
        }}

    } else {
        //size = (vectorField->getWidth() - 2)*(vectorField->getHeight() - 2)*(vectorField->getDepth() - 2);
        // Too slow to check entire image..
        size = 49*49*49;
        for(uint z = 1; z < 50;/*vectorField->getDepth()-1;*/ ++z) {
        for(uint y = 1; y < 50;/*vectorField->getHeight()-1;*/ ++y) {
        for(uint x = 1; x < 50;/*vectorField->getWidth()-1;*/ ++x) {
            Vector3i position(x, y, z);
            Vector3f inputVector = input->getVector(position).head(3);
            Vector3f resultVector = result->getVector(position).head(3);
            Vector3f laplacian = calculateLaplacian(input, result, position);
            Vector3f residual = mu*laplacian - (inputVector.x()*inputVector.x() + inputVector.y()*inputVector.y())*(resultVector - inputVector);
            sum += residual.norm();
        }}}
    }

    return sum / size;
}

TEST_CASE("Gradient vector flow with Euler method 2D 16 bit", "[fast][GVF][GradientVectorFlow][EulerGradientVectorFlow][2D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());
    gradient->set16bitStorageFormat();

    EulerGradientVectorFlow::pointer gvf = EulerGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set16bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

TEST_CASE("Gradient vector flow with Euler method 2D 32 bit", "[fast][GVF][GradientVectorFlow][EulerGradientVectorFlow][2D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());

    EulerGradientVectorFlow::pointer gvf = EulerGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set32bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

TEST_CASE("Gradient vector flow with Euler method 3D 16 bit", "[fast][GVF][GradientVectorFlow][EulerGradientVectorFlow][3D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());
    gradient->set16bitStorageFormat();

    EulerGradientVectorFlow::pointer gvf = EulerGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set16bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

TEST_CASE("Gradient vector flow with Euler method 3D 32 bit", "[fast][GVF][GradientVectorFlow][EulerGradientVectorFlow][3D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());

    EulerGradientVectorFlow::pointer gvf = EulerGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set32bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

TEST_CASE("Gradient vector flow with Multigrid method 3D 16 bit", "[fast][GVF][GradientVectorFlow][MultigridGradientVectorFlow][3D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());
    gradient->set16bitStorageFormat();

    MultigridGradientVectorFlow::pointer gvf = MultigridGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set16bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

TEST_CASE("Gradient vector flow with Multigrid method 3D 32 bit", "[fast][GVF][GradientVectorFlow][MultigridGradientVectorFlow][3D]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Ball/US-3Dt_0.mhd");

    ScaleImage::pointer normalize = ScaleImage::New();
    normalize->setInputConnection(importer->getOutputPort());

    ImageGradient::pointer gradient = ImageGradient::New();
    gradient->setInputConnection(normalize->getOutputPort());

    MultigridGradientVectorFlow::pointer gvf = MultigridGradientVectorFlow::New();
    gvf->setInputConnection(gradient->getOutputPort());
    gvf->set32bitStorageFormat();
    gvf->update();

    CHECK(calculateGVFVectorFieldResidual(gradient->getOutputData<Image>(), gvf->getOutputData<Image>(), gvf->getMuConstant())
            < 0.001);
}

}
