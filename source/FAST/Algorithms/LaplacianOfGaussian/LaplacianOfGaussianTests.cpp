#include "FAST/Testing.hpp"
#include "FAST/Algorithms/LaplacianOfGaussian/LaplacianOfGaussian.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

namespace fast {

TEST_CASE("No input given to LaplacianOfGaussian throws exception", "[fast][LaplacianOfGaussian][LoG]") {
    auto filter = LaplacianOfGaussian::create();
    CHECK_THROWS(filter->update());
}

TEST_CASE("Negative or zero sigma and mask size input throws exception in LaplacianOfGaussian" , "[fast][LaplacianOfGaussian][LoG]") {
    auto filter = LaplacianOfGaussian::create();

    CHECK_THROWS(filter->setMaskSize(-4));
    CHECK_THROWS(filter->setMaskSize(0));
    CHECK_THROWS(filter->setStandardDeviation(-4));
    CHECK_THROWS(filter->setStandardDeviation(0));
}

TEST_CASE("Even input as mask size throws exception in LaplacianOfGaussian", "[fast][LaplacianOfGaussian][LoG]") {
    auto filter = LaplacianOfGaussian::create();

    CHECK_THROWS(filter->setMaskSize(2));
}

TEST_CASE("Laplacian of Gaussian on 2D image with OpenCL", "[fast][LaplacianOfGaussian][LoG][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.jpg");

    auto filter = LaplacianOfGaussian::create(3, 9)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(filter);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->start();
}

}
