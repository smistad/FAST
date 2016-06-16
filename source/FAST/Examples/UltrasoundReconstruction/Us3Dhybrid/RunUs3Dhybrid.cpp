/**
* Examples/DataImport/streamImagesFromDisk.cpp
*
* If you edit this example, please also update the wiki and source code file in the repository.
*/
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/TestDataPath.hpp"
#include "FAST/Algorithms/UsReconstruction/Us3Dhybrid/Us3Dhybrid.hpp"

using namespace fast;

int main() {
    // Import images from files using the ImageFileStreamer
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    // The hashtag here will be replaced with an integer, starting with 0 as default
    //std::string folder = 'US-2Dt';
    //std::nameformat = 'US-2Dt_#.mhd';

    std::string folder = "/rekonstruksjons_data/US_01_20130529T084519/";
    std::string nameformat = "US_01_20130529T084519_ScanConverted_#.mhd";
    std::string input_filename = std::string(FAST_TEST_DATA_DIR) + folder + nameformat;
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    //streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    streamer->setFilenameFormat(input_filename);
    //streamer->setMaximumNumberOfFrames(746); //746 total
    streamer->setStartNumber(700);//735);//200);
    //streamer->setStepSize(3);
    //streamer->enableLooping();

    // Reconstruction PNN
    Us3Dhybrid::pointer pnnHybrid = Us3Dhybrid::New();
    pnnHybrid->setInputConnection(streamer->getOutputPort());
    pnnHybrid->setScaleToMax(400.0f);

    while (!pnnHybrid->hasCalculatedVolume()){
        //streamer->update();
        pnnHybrid->update();
    }

    // Renderer volume
    /*
    VolumeRenderer::pointer volumeRenderer = VolumeRenderer::New();
    volumeRenderer->addInputConnection(pnnHybrid->getOutputPort());
    OpacityTransferFunction::pointer otf = OpacityTransferFunction::New();
    otf->addAlphaPoint(0.0, 0.0);
    otf->addAlphaPoint(1.0, 0.5);
    ColorTransferFunction::pointer ctf = ColorTransferFunction::New();
    ctf->addRGBPoint(0.0, 0, 1, 0);
    ctf->addRGBPoint(1.0, 1, 0, 0);

    ColorTransferFunction::pointer ctf1 = ColorTransferFunction::New();
    ctf1->addRGBPoint(000.0, 1.0, 0.0, 0.0);
    ctf1->addRGBPoint(127.0, 0.0, 1.0, 0.0);
    ctf1->addRGBPoint(255.0, 0.0, 0.0, 1.0);
    OpacityTransferFunction::pointer otf1 = OpacityTransferFunction::New();
    otf1->addAlphaPoint(000.0, 0.0);
    otf1->addAlphaPoint(255.0, 1.0);

    volumeRenderer->setColorTransferFunction(0, ctf1);
    volumeRenderer->setOpacityTransferFunction(0, otf1);
    */
    /*
    //Alt for now, display image
    //Image renderer
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(pnnHybrid->getOutputPort());
    
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(imageRenderer); //volumeRenderer);//renderer);
    //window->setMaximumFramerate(10); //unngå at buffer går tomt?
    //window->set2DMode();
    window->setTimeout(5 * 1000); // automatically close window after 5 seconds
    window->start();
    */
    
    //Exporter mhd
    MetaImageExporter::pointer exporter = MetaImageExporter::New();
    std::string output_filename = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "_VOL.mhd";
    exporter->setFilename(output_filename);
    //exporter->setFilename("Output/US_01_20130529T084519_ScanConverted_volume_test.mhd");
    //exporter->setInputConnection(pnnHybrid->getOutputPort());
    //exporter->setInput(pnnHybrid);
    Image::pointer resultVolume = pnnHybrid->getStaticOutputData<Image>(0);
    ImageAccess::pointer resAccess = resultVolume->getImageAccess(ACCESS_READ);
    float * floatVolume = (float*)resAccess->get();
    std::cout << "Size of volume" << resultVolume->getSize() << std::endl;
    for (int i = 0; i < 100; i++){
        float p = floatVolume[i];
        float w = p*1.2;
    }
    exporter->setInputData(resultVolume);
    exporter->update();
}