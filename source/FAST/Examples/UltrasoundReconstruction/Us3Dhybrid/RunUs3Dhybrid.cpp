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
#include <boost/filesystem.hpp>

using namespace fast;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

int main() {
    // Import images from files using the ImageFileStreamer
    
    // The hashtag here will be replaced with an integer, starting with 0 as default
    //std::string folder = 'US-2Dt';
    //std::nameformat = 'US-2Dt_#.mhd';

    // SETTINGS
    std::string folder = "/rekonstruksjons_data/US_01_20130529T084519/";
    std::string nameformat = "US_01_20130529T084519_ScanConverted_#.mhd";
    std::string input_filename = std::string(FAST_TEST_DATA_DIR) + folder + nameformat;
    int startNumber = 735; // 400; //500; //400;//700; //200; //700; //735;
    int stepSize = 1; // 5; //3
    int scaleToMaxInt = 400; // 200; //400;
    float scaleToMax = float(scaleToMaxInt);
    float voxelSpacing = 0.5f; //0.5f; //0.2f; // 0.03 / 0.01 //dv // Større verdi gir mindre oppløsning
    float globalScaling = 1.0f;  //5.0f; //7/10 osv
    float initZSpacing = 2.0f; //1.0f // 0.2f; // 0.1f; // 0.05f; // 0.1f / 0.02f
    //initZ - større verdi gir større z-akse i volum
    float calcedDV = 0.1 * globalScaling * initZSpacing * voxelSpacing; // / 4.0;
    float setDV = calcedDV;// 0.05f;
    float maxRvalue = calcedDV * 8; //0.2f; //0.5f// 1.0f; //2.0f;// voxelSpacing * 2 * globalScaling; //*(200/globalScaling) // *globalScaling * 3;
    
    bool runVNNonly = false;
    bool runCLHybrid = false;
    bool runPNNonly = false;

    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    {
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        //streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
        streamer->setFilenameFormat(input_filename);
        //streamer->setMaximumNumberOfFrames(746); //746 total
        streamer->setStartNumber(startNumber);
        streamer->setStepSize(stepSize);
        //streamer->enableLooping();
    }

    std::string output_filename = ""; {
        // Create directory if does not exist
        std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "/";
        std::string streamStart = std::to_string(startNumber);
        std::string streamStep = std::to_string(stepSize);
        std::string streamScale = std::to_string(scaleToMaxInt);
        std::string volumeSpacing = to_string_with_precision(voxelSpacing, 3);//std::to_string(voxelSpacing);
        std::string volumeDV = to_string_with_precision(setDV, 3);
        std::string volumeRmax = to_string_with_precision(maxRvalue, 3);//std::to_string(maxRvalue);
        std::string volumeGlobalScaling = to_string_with_precision(globalScaling, 2);//std::to_string(int(globalScaling));
        std::string volumeZinitSpacing = to_string_with_precision(initZSpacing, 3); //std::to_string(initZSpacing);
        std::string runningStyle = "";
        if (runVNNonly){
            runningStyle += "VNN_";
        }
        else if (runCLHybrid){
            runningStyle += "CL_";
        }
        else if (runPNNonly){
            runningStyle += "PNN_";//std::to_string(runPNNonly);
        }
        output_filename += _filePath + "VOLUME_" + runningStyle + "start-" + streamStart + "@" + streamStep;
        output_filename += "(s" + volumeSpacing + "_gS" + volumeGlobalScaling + "_dv" + volumeDV + "_rMaz" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd";
        std::cout << "Output filename: " << output_filename << std::endl;
        //std::string output_filename = std::string(FAST_TEST_DATA_DIR) + "/output/" + "VolumeOutput.mhd";
    }

    Us3Dhybrid::pointer pnnHybrid;
    {
        // Reconstruction PNN
        pnnHybrid = Us3Dhybrid::New();
        pnnHybrid->setInputConnection(streamer->getOutputPort());
        pnnHybrid->setScaleToMax(scaleToMax);
        pnnHybrid->setVoxelSpacing(voxelSpacing);
        pnnHybrid->setDV(setDV);
        pnnHybrid->setRmax(maxRvalue);
        pnnHybrid->setGlobalScaling(globalScaling);
        pnnHybrid->setZDirInitSpacing(initZSpacing);
        //Priority VNN > PNN > CL > Normal
        pnnHybrid->setVNNrunMode(runVNNonly); //Run as VNN
        pnnHybrid->setCLrun(runCLHybrid); //Run as CL hybrid
        pnnHybrid->setPNNrunMode(runPNNonly); //Run as PNN

        //OpenCLDevice::pointer clDevice = getMainDevice();
        //clDevice->getDevice()->getInfo();
        //int error = clGetDeviceInfo(, CL_DEVICE_EXTENSIONS, none, None, None);
        while (!pnnHybrid->hasCalculatedVolume()){
            //streamer->update();
            pnnHybrid->update();
        }
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
    MetaImageExporter::pointer exporter;
    {
        exporter = MetaImageExporter::New();
        //std::string output_filename = _filePath + "VOLUME_" + runningStyle + "volSpacing#" + volumeSpacing + "_rMax#" + volumeRmax + "_start#" + streamStart + "_step#" + streamStep + "_gScale#" + volumeGlobalScaling + ".mhd";
    
        exporter->setFilename(output_filename);
        //exporter->setFilename("Output/US_01_20130529T084519_ScanConverted_volume_test.mhd");
        //exporter->setInputConnection(pnnHybrid->getOutputPort());
        //exporter->setInput(pnnHybrid);
        Image::pointer resultVolume = pnnHybrid->getStaticOutputData<Image>(0);
        /*
        ImageAccess::pointer resAccess = resultVolume->getImageAccess(ACCESS_READ);
        float * floatVolume = (float*)resAccess->get();
        std::cout << "Size of volume" << resultVolume->getSize() << std::endl;
        for (int i = 0; i < 100; i++){
            float p = floatVolume[i];
            float w = p*1.2;
        }
        */
        exporter->setInputData(resultVolume);
        exporter->update();
        std::cout << "Output filename: " << output_filename << std::endl;
    }
}