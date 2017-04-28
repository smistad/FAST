/**
 * Examples/Filtering/gaussianSmoothingFilter.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include <stdio.h>
#include <time.h>

#include "NonLocalMeans.hpp"

using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    ImageImporter::pointer importer = ImageImporter::New();
    //importer->setFilename(Config::getTestDataPath()+"/skull256.mhd");
    importer->setFilename(Config::getTestDataPath()+"/grayscaleMirror.jpg");
    
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
    int groupSize = 9;
    int windowSize = 51;
    float sigma = 0.1f;
    float dS = 1.335f;
    
    
    // Smooth image
    NonLocalMeans::pointer filter = NonLocalMeans::New();
    filter->setInputConnection(importer->getOutputPort());
    //filter->setGroupSize(15);
    //filter->setWindowSize(51);
    //filter->setSigma(0.15f);
    //filter->setDenoiseStrength(0.15f);
    filter->setGroupSize(groupSize);
    filter->setWindowSize(windowSize);
    filter->setSigma(sigma);
    filter->setDenoiseStrength(dS);
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
