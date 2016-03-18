/**
* Algorithms/Filtering/Filtering.cpp
*
* If you edit this example, please also update the wiki and source code file in the repository.
*/
#include <iostream>
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"

#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"

#include "FAST/Testing.hpp"

using namespace fast;

int main() {
    //std::string INPUT_FILENAME = "/US-2D.jpg";
    std::string INPUT_FILENAME = "/retina.png";
    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + INPUT_FILENAME);

    // Smooth image
    GaussianFiltering::pointer convolution = GaussianFiltering::New();
    convolution->setStandardDeviation(3.0);
    convolution->setMaskSize(3);
    convolution->setConvRunType(2); // 1:twopass, 2:adv, else: naive
    convolution->setInputConnection(importer->getOutputPort());
    
    SobelFiltering::pointer convolutionX = SobelFiltering::New();
    convolutionX->setDirection(0); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    convolutionX->setInputConnection(importer->getOutputPort());
    convolutionX->setConvRunType(0); // 1:twopass, 2:adv, else: naive
    

    SobelFiltering::pointer convolutionY = SobelFiltering::New();
    convolutionY->setDirection(1); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    convolutionY->setInputConnection(importer->getOutputPort());
    convolutionY->setConvRunType(0); // 1:twopass, 2:adv, else: naive

    // Average and cut image
    AverageImages::pointer averageing = AverageImages::New();
    averageing->setCutOverhead(true);
    averageing->setInputConnection(0, convolutionX->getOutputPort());
    averageing->setInputConnection(1, convolutionY->getOutputPort());
    
    // Exporter image
    ImageExporter::pointer exporter = ImageExporter::New();
    std::string output_filename = std::string(FAST_TEST_DATA_DIR) + "/output/" + INPUT_FILENAME + ".output.png";
    exporter->setFilename(output_filename);
    exporter->setInputConnection(convolution->getOutputPort());
    exporter->update();

    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(convolution->getOutputPort());
    ImageRenderer::pointer initialRenderer = ImageRenderer::New();
    initialRenderer->addInputConnection(importer->getOutputPort());
    DualViewWindow::pointer window = DualViewWindow::New();

    window->addRendererToTopLeftView(initialRenderer);
    window->addRendererToBottomRightView(renderer);
    window->setWidth(1400);
    window->setHeight(550);
    window->setTimeout(5 * 1000); // automatically close window after 5 seconds
    window->start();
}
