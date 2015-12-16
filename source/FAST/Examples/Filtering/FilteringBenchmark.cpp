//#include "FAST/Visualization/Window.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
//#include "FAST/Algorithms/Filtering/Filtering.hpp"
//#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"
#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
//#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"

#include <FAST/Importers/ImageFileImporter.hpp>
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
//#include "FAST/Exporters/ImageExporter.hpp"

#include "FAST/TestDataPath.hpp"

//#include <chrono>

//#include <thread>
//#include <boost/bind.hpp>

namespace fast {

//#define FAST_TEST_DATA_DIR "C:\Users\FAST PC 2\Ruben FAST\TestData";

void testGaussian(std::string inputImage, int runType, 
        int maskMin, int maskMax,
        float stdDevMin, float stdDevMax, float stdDevStep)
{
    std::string mFilterTypeString = "Gauss";
    std::string initialInputImageName = inputImage; // getInputFilename(inputImage);

    ImageFileImporter::pointer mImporter = ImageFileImporter::New();
    mImporter->setFilename(FAST_TEST_DATA_DIR + initialInputImageName);
    
    // Define Gaussian
    GaussianFiltering::pointer mGaussian = GaussianFiltering::New();
    mGaussian->setStandardDeviation(stdDevMin);
    mGaussian->setMaskSize(maskMin);
    mGaussian->setConvRunType(runType); // 1:twopass, 2:adv, else: naive
    mGaussian->setInputConnection(mImporter->getOutputPort());
    mGaussian->enableRuntimeMeasurements(); //for timing

    // Set up rendering
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(mGaussian->getOutputPort());
    
    // Add to view
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);

    window->setHeight(700);
    window->setWidth(700);
    window->start();


}

std::string getInputFilename(int inputnum){
    std::string INPUT_FILENAME;
    switch (inputnum){
    case 1:
        INPUT_FILENAME = "retina_big.png"; //"test.png";//
        break;
    case 2:
        INPUT_FILENAME = "cornerTest.png"; //cornerTest2.png
        break;
    default:
        INPUT_FILENAME = "US-2D.jpg";
    }
    return INPUT_FILENAME;
}

std::string numToRunType(int num){
    switch (num){
    case 0:
        return "Naive";
    case 1:
        return "Twopass";
    case 2:
        return "Advanced";
    }

    return "None";

}

int main() {
    std::string inputImage = getInputFilename(1);
    int runType = 0; // 0: naive, 1:twopass(global), 2:local
    testGaussian(inputImage, runType, 3, 7, 0.25, 10.0, 0.25);
    /*
    int initialMaskSize = 3;
    float initialStdDev = 1.0;
    int initialRunType = 2; // 0:naive, 1:Twopass
    int initialFilterType = 1; // 1:Gauss, 2:Sobel, ..

    int initialInputImage = 1; // 0:US, 1:Retina 2:CornerTest

    //
    mFilterSize = initialMaskSize;
    mRunType = initialRunType;
    mRunTypeString = numToRunType(initialRunType);
    mGaussStdDev = initialStdDev;

    // Create a 2D view
    mView = createView();
    mView->set2DMode();
    mView->setMaximumFramerate(1);
    mView->setFixedWidth(600);
    //enableFullscreen();

    //// --  Initialize Filters -- ////
    // Define Filtering (box)
    mBoxFilter = Filtering::New();

    // Define Sobel
    //SobelFiltering::pointer 
    mSobelX = SobelFiltering::New();
    mSobelX->setDirection(0); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    mSobelX->setInputConnection(mImporter->getOutputPort());
    mSobelX->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive
    mSobelX->enableRuntimeMeasurements(); //for timing

    //SobelFiltering::pointer 
    mSobelY = SobelFiltering::New();
    mSobelY->setDirection(1); // x:0, y:1, z:2 == x:horizontal, y:vertical, z:depth
    mSobelY->setInputConnection(mImporter->getOutputPort());
    mSobelY->setConvRunType(initialRunType); // 1:twopass, 2:adv, else: naive
    mSobelY->enableRuntimeMeasurements(); //for timing

    //AverageImages::pointer 
    mSobelTot = AverageImages::New();
    mSobelTot->setCutOverhead(false); //true
    mSobelTot->setInputConnection(0, mSobelX->getOutputPort());
    mSobelTot->setInputConnection(1, mSobelY->getOutputPort());
    mSobelTot->enableRuntimeMeasurements(); //for timing

    // Set up rendering
    ImageRenderer::pointer renderer = ImageRenderer::New();
    //std::string filterType;
    switch (initialFilterType){
    case 1:
        mFilterTypeString = "Gauss";
        mOutPort = mGaussian->getOutputPort();
        mFilterType = 1;
        break;
    case 2:
        mFilterTypeString = "Sobel";
        mOutPort = mSobelTot->getOutputPort();
        //renderer->addInputConnection(mSobelTot->getOutputPort());
        mFilterType = 2;
        break;
    default:
        mFilterTypeString = "Avg";
        mOutPort = mBoxFilter->getOutputPort();
        mFilterType = 0;
        //renderer->addInputConnection(mBoxFilter->getOutputPort());
    }
    renderer->addInputConnection(mOutPort);
    // Add to view
    mView->addRenderer(renderer);

    window->setHeight(700);
    window->start();

    */

    return 0;
}

}