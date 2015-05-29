#include "FAST/Tests/catch.hpp"
#include "FAST/Algorithms/LaplacianOfGaussian/LaplacianOfGaussian.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

namespace fast {

TEST_CASE("No input given to LaplacianOfGaussian throws exception", "[fast][LaplacianOfGaussian][LoG]") {
    LaplacianOfGaussian::pointer filter = LaplacianOfGaussian::New();
    CHECK_THROWS(filter->update());
}

TEST_CASE("Negative or zero sigma and mask size input throws exception in LaplacianOfGaussian" , "[fast][LaplacianOfGaussian][LoG]") {
    LaplacianOfGaussian::pointer filter = LaplacianOfGaussian::New();

    CHECK_THROWS(filter->setMaskSize(-4));
    CHECK_THROWS(filter->setMaskSize(0));
    CHECK_THROWS(filter->setStandardDeviation(-4));
    CHECK_THROWS(filter->setStandardDeviation(0));
}

TEST_CASE("Even input as mask size throws exception in LaplacianOfGaussian", "[fast][LaplacianOfGaussian][LoG]") {
    LaplacianOfGaussian::pointer filter = LaplacianOfGaussian::New();

    CHECK_THROWS(filter->setMaskSize(2));
}

TEST_CASE("Laplacian of Gaussian on 2D image with OpenCL", "[fast][LaplacianOfGaussian][LoG][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");

    LaplacianOfGaussian::pointer filter = LaplacianOfGaussian::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->setMaskSize(9);
    filter->setStandardDeviation(3);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->start();
}

}
