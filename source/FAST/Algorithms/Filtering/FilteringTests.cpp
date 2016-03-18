/**
* Algorithms/Filtering/Filtering.cpp
*
* If you edit this example, please also update the wiki and source code file in the repository.
*/
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"

using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "/US-2D.jpg");


    // Smooth image
    Filtering::pointer convolution = Filtering::New();
    convolution->setInputConnection(importer->getOutputPort());
    //convolution->setStandardDeviation(2.0);
    convolution->setMaskSize(5);

    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(convolution->getOutputPort());
    ImageRenderer::pointer initialRenderer = ImageRenderer::New();
    initialRenderer->addInputConnection(importer->getOutputPort());
    DualViewWindow::pointer window = DualViewWindow::New();
    /*window->addRenderer(renderer);*/
    window->addRendererToTopLeftView(initialRenderer);
    window->addRendererToBottomRightView(renderer);
    window->setWidth(1400);
    window->setHeight(550);
    window->setTimeout(5 * 1000); // automatically close window after 5 seconds
    window->start();
}
