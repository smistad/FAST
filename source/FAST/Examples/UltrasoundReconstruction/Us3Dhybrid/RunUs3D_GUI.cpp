/**
* Examples/UltrasoundReconstruction/Us3Dhybrid/RunUs3D_GUI.cpp
*
* If you edit this example, please also update the wiki and source code file in the repository.
*/
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/TestDataPath.hpp"
#include "FAST/Algorithms/UsReconstruction/Us3Dhybrid/Us3Dhybrid.hpp"

#include "UsGUI.hpp"

using namespace fast;

/*
enum Us3DRunMode {
    clHybrid = 0,
    cpuHybrid = 1,
    clVNN = 2,
    cpuVNN = 3,
    clPNN = 4,
    cpuPNN = 5
};*/

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

void runAlgorithmAndExportImage(
    float setDV, float maxRvalue, 
    std::string input_filename, std::string nameformat, std::string output_subfolder = "",
    int volSizeM = 32, float initZspacing = 1.0f,
    Us3DRunMode runType = Us3DRunMode::clHybrid,
    int startNumber = 0, int stepSize = 1
    ){

    if (runType == Us3DRunMode::clVNN || runType == Us3DRunMode::clPNN){
        return; //Unimplemented runType
    }
    std::cout << "## RUNNING with settings - setDV: " << setDV << " & rMax: " << maxRvalue << " ##" << std::endl;
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    {
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        streamer->setFilenameFormat(input_filename);
        streamer->setStartNumber(startNumber);
        streamer->setStepSize(stepSize);
    }

    std::string output_filename = ""; {
        // Create directory if does not exist
        std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "/" + output_subfolder;
        std::string streamStart = std::to_string(startNumber);
        std::string streamStep = std::to_string(stepSize);
        std::string volumeDV = to_string_with_precision(setDV, 3);
        std::string volumeRmax = to_string_with_precision(maxRvalue, 3);
        std::string volumeSizeMillion = std::to_string(volSizeM);
        std::string volumeZinitSpacing = to_string_with_precision(initZspacing, 3);
        std::string runningStyle = "";
        switch (runType){
            case clHybrid :
                runningStyle += "CL_";
                break;
            case cpuHybrid :
                break;
            case cpuVNN :
                runningStyle += "VNN_";
                break;
            case cpuPNN :
                runningStyle += "PNN_";
                break;
            default:
                std::cout << "Run type " << (Us3DRunMode)runType << " is not implemented. Quitting.." << std::endl;
                return;
        }

        output_filename += _filePath + "VOLUME_" + runningStyle + volumeSizeMillion + "M_" + "start-" + streamStart + "@" + streamStep;
        output_filename += "(dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd";
        std::cout << "Output filename: " << output_filename << std::endl;
    }

    Us3Dhybrid::pointer pnnHybrid;
    {
        // Reconstruction PNN
        pnnHybrid = Us3Dhybrid::New();
        pnnHybrid->setInputConnection(streamer->getOutputPort());
        pnnHybrid->setDV(setDV);
        pnnHybrid->setRmax(maxRvalue);
        pnnHybrid->setVolumeSize(volSizeM);
        pnnHybrid->setZDirInitSpacing(initZspacing);
        //Priority VNN > PNN > CL > Normal
        pnnHybrid->setRunMode(runType);

        while (!pnnHybrid->hasCalculatedVolume()){
            pnnHybrid->update();
        }
    }
}

void runAlgorithmAndExportImageOld(
    float setDV, float maxRvalue,
    std::string input_filename, std::string nameformat, float voxelSpacing = 0.1f, std::string testPlace = "",
    int startNumber = 0, int stepSize = 1, int volumeSizeMil = 32, float initZSpacing = 1.0f,
    bool runVNNonly = false, bool runCLHybrid = true, bool runPNNonly = false
    ){

    //float globalScaling,
    std::cout << "## RUNNING with settings - setDV: " << setDV << " & rMax: " << maxRvalue << " ##" << std::endl;
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    {
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        streamer->setFilenameFormat(input_filename);
        streamer->setStartNumber(startNumber);
        streamer->setStepSize(stepSize);
    }

    std::string output_filename = ""; {
        // Create directory if does not exist
        std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "/" + testPlace;
        std::string streamStart = std::to_string(startNumber);
        std::string streamStep = std::to_string(stepSize);
        //std::string streamScale = std::to_string(scaleToMaxInt);
        std::string volumeSpacing = to_string_with_precision(voxelSpacing, 3);//std::to_string(voxelSpacing);
        std::string volumeDV = to_string_with_precision(setDV, 3);
        std::string volumeRmax = to_string_with_precision(maxRvalue, 3);//std::to_string(maxRvalue);
        std::string volumeSizeMillion = std::to_string(volumeSizeMil);//to_string_with_precision(maxRvalue, 3);
        //std::string volumeGlobalScaling = to_string_with_precision(globalScaling, 2);//std::to_string(int(globalScaling));
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
        output_filename += _filePath + "VOLUME_" + runningStyle + volumeSizeMillion + "M_" + "start-" + streamStart + "@" + streamStep;// +".mhd";
        //output_filename += "(s" + volumeSpacing + "_dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd";
        output_filename += "(dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd"; //s" + volumeSpacing + "_
        //+ "_gS" + volumeGlobalScaling
        std::cout << "Output filename: " << output_filename << std::endl;
    }

}

int main() {
    UsGUI::pointer window = UsGUI::New();

    window->start();

    return 0;
}