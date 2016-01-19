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
    //importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "/US-1-2D.png");
    
    /*
    //K = 1, dvs K = gamle versjonen, M_PI gaus
    int groupSize = 5;
    int windowSize = 51;
    float sigma = 1.65f;
    float dS = 0.5f;
    */
    /*
    //K = 0, dvs K = 1/2d
    int groupSize = 5;
    int windowSize = 51;
    float sigma = 0.65f;
    float dS = 0.135f;
    */
    //K = 3 dvs ny gauss
    int groupSize = 17;
    int windowSize = 17;
    float sigma = 5.000f;
    //float dS = 0.03999f;
    float dS = 5.0f;
    
    // Smooth image
    NoneLocalMeans::pointer filter = NoneLocalMeans::New();
    filter->setInputConnection(importer->getOutputPort());
    //filter->setGroupSize(15);
    //filter->setWindowSize(51);
    //filter->setSigma(0.15f);
    //filter->setDenoiseStrength(0.15f);
    filter->setGroupSize(groupSize);
    filter->setWindowSize(windowSize);
    filter->setSigma(sigma);
    filter->setDenoiseStrength(dS);
    filter->setEuclid(1);
    filter->setK(2);
    //filter->setOutputType(TYPE_FLOAT);
    
    //Export img
    ImageExporter::pointer exporter = ImageExporter::New();
    std::string text = "";
    //text += std::to_string( groupSize );
    //text += "WS_";
    //text += std::to_string( windowSize );
    //text += "S_";
    //text += std::to_string( sigma );
    //text += "DS_";
    //text += std::to_string( dS );
    text += "TIME_";
    time_t seconds;
    seconds = time (NULL);
    text += std::to_string( seconds );
    text += ".jpg";
    exporter->setFilename(text);
    exporter->setInputConnection(filter->getOutputPort());
    exporter->update();
    
    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(200*1000); // automatically close window after 5 seconds
    window->start();
}
