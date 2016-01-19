/**
 * Examples/Filtering/gaussianSmoothingFilter.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Algorithms/NoneLocalMeans/NoneLocalMeans.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/TestDataPath.hpp"
#include <stdio.h>
#include <time.h>

using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"/US-2D.jpg");

    // Some parameters
    int groupSize = 17;
    int windowSize = 17;
    float sigma = 5.000f;
    float dS = 5.0f;
    
    // Smooth image
    NoneLocalMeans::pointer filter = NoneLocalMeans::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->setGroupSize(groupSize);
    filter->setWindowSize(windowSize);
    filter->setSigma(sigma);
    filter->setDenoiseStrength(dS);
    filter->setEuclid(1);
    filter->setK(2);
    
    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(200*1000); // automatically close window after 5 seconds
    window->start();
}
