/**
* Examples/UltrasoundReconstruction/Us3Dhybrid/RunUs3Dhybrid.cpp
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

std::string breakString = "\n\n";
void runAlgorithmAndExportImage(
    float setDV, float maxRvalue,
    std::string input_filename, std::string nameformat, std::string output_subfolder, std::string nickname,
    int volSizeM, float initZspacing, int HF_gridSize, bool HF_progressive,
    Us3DRunMode runType, bool hybridWeightGaussian,
    int startNumber, int stepSize, int verbosity
    ){

    if (runType == Us3DRunMode::clVNN){// || runType == Us3DRunMode::clPNN){
        return; //Unimplemented runType
    }
    if (verbosity >= 3){
        std::cout << "## RUNNING with settings - setDV: " << setDV << " & rMax: " << maxRvalue << " ##" << std::endl;
    }
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    {
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        streamer->setFilenameFormat(input_filename);
        streamer->setStartNumber(startNumber);
        streamer->setStepSize(stepSize);
    }

    std::string output_filename = ""; {
        // Create directory if does not exist
        std::string folder = nameformat;
        if (nickname != ""){ folder = nickname + "/" + nameformat; }
        std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + folder + "/" + output_subfolder;
        std::string streamStart = std::to_string(startNumber);
        std::string streamStep = std::to_string(stepSize);
        std::string volumeDV = to_string_with_precision(setDV, 3);
        std::string volumeRmax = to_string_with_precision(maxRvalue, 3);
        std::string volumeSizeMillion = std::to_string(volSizeM);
        std::string volumeZinitSpacing = to_string_with_precision(initZspacing, 3);
        std::string volumeHFgridSize = std::to_string(HF_gridSize);
        std::string runningStyle = "";
        switch (runType){
        case Us3DRunMode::clHybrid:
            runningStyle += "CL_";
            if (hybridWeightGaussian){ runningStyle += "gauss_"; }
            else { runningStyle += "linear_"; }
            break;
        case Us3DRunMode::cpuHybrid:
            break;
        case Us3DRunMode::cpuVNN:
            runningStyle += "VNN_";
            break;
        case Us3DRunMode::cpuPNN:
            runningStyle += volumeHFgridSize+"_PNN_"; //HF prog here?
            break;
        case Us3DRunMode::clPNN:
            runningStyle += volumeHFgridSize+"_PNN-CL_";
            if (HF_progressive){ runningStyle += "prog_";  }
            break;
        case Us3DRunMode::alphaCLHybrid:
            runningStyle += "alphaCL_";
            if (hybridWeightGaussian){ runningStyle += "gauss_"; }
            else { runningStyle += "linear_"; }
            break;
        case Us3DRunMode::alphaCLPNN:
            runningStyle += volumeHFgridSize + "_PNN-alphaCL_"; 
            break;
        default:
            std::cout << "Run type " << (Us3DRunMode)runType << " is not implemented. Quitting.." << std::endl;
            return;
        }

        output_filename += _filePath + "VOL_" + runningStyle + volumeSizeMillion + "M_" + nickname;
        output_filename += "(dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + "_start" + streamStart + "@" + streamStep + ")" + ".mhd";
        if (verbosity >= 7){
            std::cout << "Output filename: " << output_filename << std::endl;
        }
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
        pnnHybrid->setHFgridSize(HF_gridSize);
        pnnHybrid->setHFprogressive(HF_progressive);
        //Priority VNN > PNN > CL > Normal
        pnnHybrid->setRunMode(runType);
        pnnHybrid->setGaussianWeightMode(hybridWeightGaussian);
        pnnHybrid->setVerbosity(verbosity);

        while (!pnnHybrid->hasCalculatedVolume()){
            pnnHybrid->update();
        }
    }

    MetaImageExporter::pointer exporter;
    {
        exporter = MetaImageExporter::New();
        exporter->setFilename(output_filename);
        exporter->enableCompression();
        Image::pointer resultVolume = pnnHybrid->getStaticOutputData<Image>(0);

        exporter->setInputData(resultVolume);
        exporter->update();
        if (verbosity >= 3){
            std::cout << "Output filename: " << output_filename << std::endl;
        }
    }

    if (verbosity >= 3){
        std::cout << breakString;
    }
    
}

void runAlgorithmAndExportImage(
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

    Us3Dhybrid::pointer pnnHybrid;
    {
        // Reconstruction PNN
        pnnHybrid = Us3Dhybrid::New();
        pnnHybrid->setInputConnection(streamer->getOutputPort());
        //pnnHybrid->setScaleToMax(scaleToMax);
        pnnHybrid->setVoxelSpacing(voxelSpacing);
        pnnHybrid->setDV(setDV);
        pnnHybrid->setRmax(maxRvalue);
        pnnHybrid->setVolumeSize(volumeSizeMil);
        //pnnHybrid->setGlobalScaling(globalScaling);
        pnnHybrid->setZDirInitSpacing(initZSpacing);
        //Priority VNN > PNN > CL > Normal
        pnnHybrid->setVNNrunMode(runVNNonly); //Run as VNN
        pnnHybrid->setCLrun(runCLHybrid); //Run as CL hybrid
        pnnHybrid->setPNNrunMode(runPNNonly); //Run as PNN

        while (!pnnHybrid->hasCalculatedVolume()){
            pnnHybrid->update();
        }
    }

    MetaImageExporter::pointer exporter;
    {
        exporter = MetaImageExporter::New();
        exporter->setFilename(output_filename);
        exporter->enableCompression();
        Image::pointer resultVolume = pnnHybrid->getStaticOutputData<Image>(0);

        exporter->setInputData(resultVolume);
        exporter->update();
        std::cout << "Output filename: " << output_filename << std::endl;
    }
}

/* TO DISABLE OPEN_CL CACHE (not just CUDA) */
void enable_cuda_build_cache(bool enable)
{
#ifdef _MSC_VER
    if (enable)
        _putenv("CUDA_CACHE_DISABLE=0");
    else
        _putenv("CUDA_CACHE_DISABLE=1");
#else // GCC
    if (enable)
        putenv("CUDA_CACHE_DISABLE=0");
    else
        putenv("CUDA_CACHE_DISABLE=1");
#endif
}

void printRunSettings(Vector4i runSetting){
    Us3DRunMode runMode = (Us3DRunMode)runSetting(0);
    int volSizeM = runSetting(1);
    int rMax = runSetting(2);
    int holeFill_gridSize = 3;
    bool runHybridWeightGaussian = false;
    std::string runModeString = "";
    if (runMode == Us3DRunMode::clPNN){
        holeFill_gridSize = runSetting(3);
        runModeString = "PNN";
        runModeString += "_" + std::to_string(holeFill_gridSize);
    }
    else if (runMode == Us3DRunMode::clHybrid || runMode == Us3DRunMode::alphaCLHybrid){
        runModeString = "Hybrid";
        if (runMode == Us3DRunMode::alphaCLHybrid){ runModeString += "_alpha"; }
        if (runSetting(3) == 1){
            runHybridWeightGaussian = true; //use gaussian weighting
            runModeString += "_gaussian";
        }
        runModeString += "_" + std::to_string(rMax);
    }
    std::cout << " - RunMode: " << runModeString << " - " << volSizeM << "M - " << std::endl;
}

void runPerformanceTests(){

    clock_t testStart = clock();
    /* ### SETTINGS ### */
    int startNumber = 0;
    int stepSize = 1;
    int verbosity = 2;
    int maxIterations = 3;
    std::string sub_folder = "performanceTests/";

    // SET 1 : 72-104535
    std::string set1folder = "Ultrasound Data Sets 2/072_Tumor_OK.cx3/072_Tumor_OK.cx3/US_Acq/US-Acq_02_19700101T104535/";
    std::string set1nameformat = "US-Acq_02_19700101T104535_Tissue_#.mhd";
    std::string set1nickname = "set-72_104535";
    float set1initZSpacing = 0.1f;//??
    
    // SET 2 : 84-103031
    std::string set2folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_03_19700101T103031/";
    std::string set2nameformat = "US-Acq_03_19700101T103031_Tissue_#.mhd";
    std::string set2nickname = "set-84_103031";
    float set2initZSpacing = 0.1f;//??

    //Run modes
    std::vector<Vector4i> runAlgorithms = {};

    Vector4i gauss8 = Vector4i(Us3DRunMode::clHybrid, 32, 8, 1); //type, M, Rmax, HF/(gauss/lin)
    Vector4i linear8 = Vector4i(Us3DRunMode::clHybrid, 32, 8, 0);
    Vector4i linear8_256 = Vector4i(Us3DRunMode::clHybrid, 256, 8, 0);
    Vector4i linear16 = Vector4i(Us3DRunMode::clHybrid, 32, 16, 0);
    Vector4i alphaLin8 = Vector4i(Us3DRunMode::alphaCLHybrid, 32, 8, 0);
    Vector4i pnn5 = Vector4i(Us3DRunMode::clPNN, 32, 0, 5);
    Vector4i pnn7 = Vector4i(Us3DRunMode::clPNN, 32, 0, 7);
    Vector4i pnn7_256 = Vector4i(Us3DRunMode::clPNN, 256, 0, 7);
    {
        runAlgorithms.push_back(gauss8);
        runAlgorithms.push_back(linear8);
        runAlgorithms.push_back(linear8_256);
        runAlgorithms.push_back(linear16);
        runAlgorithms.push_back(alphaLin8);
        runAlgorithms.push_back(pnn5);
        runAlgorithms.push_back(pnn7);
        runAlgorithms.push_back(pnn7_256);
    }

    /* ### Initialization ### */
    enable_cuda_build_cache(false); //to ensure consistent initialization times

    /* ### Run loop ### */
    for (int setNR = 1; setNR <= 2; setNR++){
        std::string folder = "";
        std::string nameformat = "";
        std::string nickname = "";
        float initZspacing = 0.1f;
        if (setNR == 1){
            folder = set1folder;
            nameformat = set1nameformat;
            nickname = set1nickname;
            initZspacing = set1initZSpacing;
        }
        else if (setNR == 2){
            folder = set2folder;
            nameformat = set2nameformat;
            nickname = set2nickname;
            initZspacing = set2initZSpacing;
        }
        std::string input_filename = std::string(FAST_TEST_DATA_DIR) + folder + nameformat;
        std::cout << "\n\n ### SET " << setNR << " running: " << nickname << "! ### \n" << std::endl;
        for (int runSettingNR = 0; runSettingNR < runAlgorithms.size(); runSettingNR++){ //runAlgorithms.size() //1
            Vector4i runSetting = runAlgorithms[runSettingNR];
            Us3DRunMode runMode = (Us3DRunMode)runSetting(0);
            int volSizeM = runSetting(1);
            int rMax = runSetting(2);
            int holeFill_gridSize = 3;
            bool runHybridWeightGaussian = false;
            if (runMode == Us3DRunMode::clPNN){
                holeFill_gridSize = runSetting(3);
            }
            else {
                if (runSetting(3) == 1){
                    runHybridWeightGaussian = true; //use gaussian weighting
                }
            }
            printRunSettings(runSetting);
            //std::cout << " - RunMode: " << runMode << " - " << volSizeM << "M - " << " - " << std::endl;
            for (int iteration = 0; iteration < maxIterations; iteration++){
                runAlgorithmAndExportImage(
                    1.0f, rMax,
                    input_filename, nameformat, sub_folder, nickname,
                    volSizeM, initZspacing, holeFill_gridSize, false,
                    runMode, runHybridWeightGaussian,
                    startNumber, stepSize, verbosity
                    );
            }
            clock_t testCurrentTime = clock();
            clock_t clockTicksTaken = testCurrentTime - testStart;
            double timeInSeconds = clockTicksTaken / (double)CLOCKS_PER_SEC;
            std::cout << " ## Time used so far " << timeInSeconds << "sec! ## \n" << std::endl;
        }
    }

    std::cout << " # # # FINISHED! # # # " << std::endl;
    Sleep(600 * 1000);
}


int main() {

    runPerformanceTests();
    return 0;
    // Import images from files using the ImageFileStreamer
    
    // The hashtag here will be replaced with an integer, starting with 0 as default
    //std::string folder = 'US-2Dt';
    //std::nameformat = 'US-2Dt_#.mhd';

    // SETTINGS
    int startNumber = 0;// 735// 400; //500; //400;//700; //200; //700; //735;
    int stepSize = 1; // 5; //3
    int scaleToMaxInt = 400; // 200; //400;
    float scaleToMax = float(scaleToMaxInt);
    float globalScaling = 1.0f;  //5.0f; //7/10 osv
    float initZSpacing = 1.0f; //0.3f;// 0.5f; // 0.2f; //0.3f seems fine //0.5f // 2.0f;          //1.0f // 0.2f; // 0.1f; // 0.05f; // 0.1f / 0.02f
    //initZ - større verdi gir større z-akse i volum
    float dvConstant = 2 * 0.15f; //0.2f ev (0.5f/3.0f)~=0.1667..
    float voxelSpacing = 0.2f;// 0.15f; //0.1f; //0.5f; //0.2f; // 0.03 / 0.01 //dv // Større verdi gir mindre oppløsning
    float RmaxMultiplier = 10.0f;
    int volumeSizeMillions = 256;// 32; // 4; // 32;// 256; // 32; // 128;// 256; // 256;// 128;// 256;// 32;// 128;  //crash at 512
    int holeFill_gridSize = 7;// 13;
    bool holeFill_progressive = false; //true;
    int verbosity = 4;

    int runInputSet = 7;// 1; //1/2
    std::string folder = "";
    std::string nameformat = "";
    std::string nickname = "";
    if (runInputSet == 0){
        folder = "/rekonstruksjons_data/US_01_20130529T084519/";
        nameformat = "US_01_20130529T084519_ScanConverted_#.mhd";
        nickname = "set-19SC";
        voxelSpacing = 0.239013f; // 0.1f;  0.15f;
        initZSpacing = 0.1f;// 0.6f;// 0.5f;//0.05f;// 1.0f;
        dvConstant = 0.5f;// 1.0f;// 0.3f;// 0.15f; //0.30f;
        RmaxMultiplier = 10.0f;
    }
    else if (runInputSet == 1){
        folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_01_19700101T102623/"; //HAS CORRUPT EXAMPLE
        nameformat = "US-Acq_01_19700101T102623_Tissue_#.mhd";
        nickname = "set-84_102623";
        voxelSpacing = 0.15f; //0.1f;
        initZSpacing = 0.1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 15.0f; // 8.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 2){ //NOT usable??
        folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_02_19700101T102827/";
        nameformat = "US-Acq_02_19700101T102827_Tissue_#.mhd";
        nickname = "set-84_102827";
        voxelSpacing = 0.1f; // 0.15f; //0.1f;
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 8.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 3){
        folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_03_19700101T103031/";
        nameformat = "US-Acq_03_19700101T103031_Tissue_#.mhd";
        nickname = "set-84_103031";
        voxelSpacing = 0.1f; // 0.15f; //0.1f;
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 4.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 4){
        folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_04_19700101T115706/";
        nameformat = "US-Acq_04_19700101T115706_Tissue_#.mhd";
        nickname = "set-84_115706";
        voxelSpacing = 0.1f; // 0.15f; //0.1f;
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 8.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 5){
        folder = "Ultrasound Data Sets 2/084_Tumor_OK.cx3/084_Tumor_OK.cx3/US_Acq/US-Acq_06_19700101T122413/";
        nameformat = "US-Acq_06_19700101T122413_Tissue_#.mhd";
        nickname = "set-84_122413";
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 4.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 6){
        folder = "Ultrasound Data Sets 2/071_Tumor.cx3/071_Tumor.cx3/US_Acq/US-Acq_01_19700101T103046/";
        //US-Acq_01_19700101T103046_Tissue_# ?? like example output?
        nameformat = "US-Acq_01_19700101T103046_Tissue_#.mhd";
        //nameformat = "US-Acq_01_19700101T103046_ScanConverted_#.mhd";
        nickname = "set-71_103046";
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 8.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    else if (runInputSet == 7){
        folder = "Ultrasound Data Sets 2/072_Tumor_OK.cx3/072_Tumor_OK.cx3/US_Acq/US-Acq_02_19700101T104535/";
        //nameformat = "US-Acq_02_19700101T104535_ScanConverted_#.mhd"; //corrupt #25..
        nameformat = "US-Acq_02_19700101T104535_Tissue_#.mhd";
        nickname = "set-72_104535";
        initZSpacing = 0.1f;//0.05f;// 1f; //0.2f;
        dvConstant = 1.0f; // 0.30f; //0.5f
        RmaxMultiplier = 8.0f;// 45.0f;// 25.0f;// 10.0f;
    }
    

    std::string input_filename = std::string(FAST_TEST_DATA_DIR) + folder + nameformat;
    float calcedDV = 0.5f / (3.0f*voxelSpacing) * globalScaling * initZSpacing; //was 5.0f // 0.1 *  // / 4.0;
    
    float calcedDV2 = dvConstant * (1.0f / voxelSpacing) * initZSpacing; //*globalScaling
    float calcedDV3 = dvConstant;
    float setDVsuggestion = calcedDV3;// 3.0f;// calcedDV2;// 0.1f; // 0.25f; //0.5f; //0.2f; //0.5f; // calcedDV;// 0.05f;
    float maxRvalueSuggestion = setDVsuggestion * RmaxMultiplier;// 10.0f;// setDVsuggestion * 5;// 10; //8; //0.2f; //0.5f// 1.0f; //2.0f;// voxelSpacing * 2 * globalScaling; //*(200/globalScaling) // *globalScaling * 3;
    bool runVNNonly = false;
    bool runCLHybrid = true; //false;
    bool runPNNonly = false;
    Us3DRunMode runMode = Us3DRunMode::clHybrid; // cpuVNN; //clPNN; //cpuVNN; //cpuHybrid; // clHybrid;
    bool runHybridWeightGaussian = true;

    bool singleTest = true;//false;

    if (!singleTest){
        std::string testPlace = "testRun6/"; //"test-clHybrid-gaussian/";
        float dvStart = 1.0f; // 0.5f; // 0.5f;
        float dvEnd = 1.1f;// 1.0f;
        float calcedDVstep = 0.5f;// calcedDV / 5.0f;
        int rStart = 4;
        int rEnd = 12;
        int rStep = 4;
        float rMaxMaximum = 30.0f;
        int dvValues = 1 +((dvEnd - dvStart) / calcedDVstep); //+1?
        int rMaxValues = 1 +((rEnd - rStart) / rStep); //+1?
        int totalRuns = dvValues * rMaxValues;
        std::cout << " ##### RUNNING MANY TESTS ##### " << std::endl;
        std::cout << " Testing with " << dvValues << " values of dv and " << rMaxValues << " values of rMax!" << std::endl;
        std::cout << " For a total max of " << totalRuns << " runs!" << std::endl;

        clock_t startTests = clock();
        int testsIt = 0;
        for (int rm = 0; rm < 2; rm++){
            if (rm == 0){
                runMode = Us3DRunMode::clHybrid;
            }
            else{
                runMode = Us3DRunMode::alphaCLHybrid;
            }
            //runMode = Us3DRunMode::clHybrid;
            for (int i = 0; i < 2; i++){ //2
                if (i == 0){
                    runHybridWeightGaussian = true;
                }
                else{
                    runHybridWeightGaussian = false;
                }
                for (float setDV = dvStart; setDV <= dvEnd; setDV += calcedDVstep){
                    for (int rMultiplier = rStart; rMultiplier <= rEnd; rMultiplier += rStep){
                        float maxRvalue = setDV * rMultiplier;
                        if (maxRvalue > rMaxMaximum)
                            continue;
                        runAlgorithmAndExportImage(
                            setDV, maxRvalue,
                            input_filename, nameformat, testPlace, nickname,
                            volumeSizeMillions, initZSpacing, holeFill_gridSize, holeFill_progressive,
                            runMode, runHybridWeightGaussian,
                            startNumber, stepSize, verbosity
                            );
                        testsIt++;
                        /*
                        runAlgorithmAndExportImage(
                        setDV, maxRvalue,
                        input_filename, nameformat, voxelSpacing, testPlace,
                        startNumber, stepSize, volumeSizeMillions, initZSpacing,
                        runVNNonly, runCLHybrid, runPNNonly
                        );
                        */
                    }
                }
            }
        }
        
        if (true){
            // RUN PNN tests
            for (int gridSize = 3; gridSize <= 7; gridSize += 2){
                runAlgorithmAndExportImage(
                    0.0f, 0.0f,
                    input_filename, nameformat, testPlace, nickname,
                    volumeSizeMillions, initZSpacing, gridSize, false,
                    Us3DRunMode::clPNN, runHybridWeightGaussian,
                    startNumber, stepSize, verbosity
                    );
                testsIt++;
            }
            // RUN PNN progressive tests
            runAlgorithmAndExportImage(
                0.0f, 0.0f,
                input_filename, nameformat, testPlace, nickname,
                volumeSizeMillions, initZSpacing, 7, true,
                Us3DRunMode::clPNN, runHybridWeightGaussian,
                startNumber, stepSize, verbosity
                );
            testsIt++;
        }
        
        clock_t endTests = clock();
        clock_t clockTicksTakenLoop = endTests - startTests;
        double timeInSecondsLoop = clockTicksTakenLoop / (double)CLOCKS_PER_SEC;
        std::cout << " - - - " << std::endl;
        std::cout << "It took " << timeInSecondsLoop << "sec to test run all "<< testsIt <<" configs!" << std::endl;

    }
    else {
        //runAlgorithmAndExportImage(setDVsuggestion, maxRvalueSuggestion, input_filename, nameformat, voxelSpacing); //Runs CL
        /*
        runAlgorithmAndExportImage(
            setDVsuggestion, maxRvalueSuggestion, 
            input_filename, nameformat, voxelSpacing, "testTwoArrays/",
            startNumber, stepSize, volumeSizeMillions, initZSpacing,
            runVNNonly, runCLHybrid, runPNNonly
            );
        */
        runAlgorithmAndExportImage(
            setDVsuggestion, maxRvalueSuggestion,
            input_filename, nameformat, "", nickname,
            volumeSizeMillions, initZSpacing, holeFill_gridSize, holeFill_progressive,
            runMode, runHybridWeightGaussian,
            startNumber, stepSize, verbosity
            );
        
    }
}


/*
float setDV = setDVsuggestion;
float maxRvalue = maxRvalueSuggestion;
std::cout << "## RUNNING with settings - setDV: " << setDV << " & rMax: " << maxRvalue << " ##" << std::endl;
ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
{
streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
//streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
streamer->setFilenameFormat(input_filename);
streamer->setStartNumber(startNumber);
streamer->setStepSize(stepSize);
}

std::string output_filename = ""; {
// Create directory if does not exist
std::string _filePath = std::string(FAST_TEST_DATA_DIR) + "/output/" + nameformat + "/";// +testPlace;
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
output_filename += "(s" + volumeSpacing + "_gS" + volumeGlobalScaling + "_dv" + volumeDV + "_rMax" + volumeRmax + "_z" + volumeZinitSpacing + ")" + ".mhd";
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
*
exporter->setInputData(resultVolume);
exporter->update();
std::cout << "Output filename: " << output_filename << std::endl;
}
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