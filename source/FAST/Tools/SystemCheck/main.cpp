#include <FAST/DeviceManager.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>
#include <FAST/Config.hpp>
#include <FAST/Visualization/Window.hpp>
#include <FAST/Importers/ImageImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <FASTVersion.hpp>

using namespace fast;

std::string doCheck() {
    // Loop over all platforms
    auto deviceManager = DeviceManager::getInstance();
    auto platforms = deviceManager->getPlatforms(DEVICE_PLATFORM_ANY);
    std::stringstream message;
    message << "FAST " << getVersion() << " SYSTEM CHECK\n\n";

    bool canVisualize = Config::getVisualization();

    if(platforms.empty()) {
        if(canVisualize) {
            std::string msg = "No OpenCL platforms found! Please install an OpenCL platform. <br>"
                              "See <a href=\"https://fast-imaging.github.io/requirements.html\">https://fast-imaging.github.io/requirements.html</a> for more information.";
            auto button = QMessageBox::critical(nullptr, "No OpenCL platform found.", msg.c_str());
        } else {
            message << "No OpenCL platforms found! Please install an OpenCL platform. "
                       "See https://fast-imaging.github.io/requirements.html for more information.";
            std::cout << message.str() << std::endl;
        }
        exit(0);
    }
    // Show information on every device in platform
    for(auto&& platform : platforms) {
        message << "OpenCL Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
        message << "========================================\n";
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for(auto&& device : devices) {
            message << device.getInfo<CL_DEVICE_VENDOR>() << ": " << device.getInfo<CL_DEVICE_TYPE>() << " - " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            message << "-----------------------------------------" << std::endl;
            message << "Writing to 3D textures: ";
            if(device.getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos) {
                message << "SUPPORTED" << std::endl;
            } else {
                message << "NOT SUPPORTED" << std::endl;
            }
            message << "OpenGL interopability: ";
            bool interopSupported = false;
            try {
                interopSupported = deviceManager->deviceHasOpenGLInteropCapability(device, platform);
            } catch(Exception &e) {
            }
            if(interopSupported) {
                message << "SUPPORTED" << std::endl;
            } else {
                message << "NOT SUPPORTED" << std::endl;
            }
        }
        message << std::endl;
    }
    // Show FAST preferred platform and device
    message << "FAST preferred platform and device" << std::endl;
    message << "========================================" << std::endl;
    auto device = std::dynamic_pointer_cast<OpenCLDevice>(deviceManager->getDefaultDevice());
    message << device->getPlatform().getInfo<CL_PLATFORM_NAME>() << " - " << device->getDevice().getInfo<CL_DEVICE_NAME>() << std::endl;
    message << std::endl;

    // Check if inference engines are available
    auto availableInferenceEngines = InferenceEngineManager::getEngineList();
    message << "Inference Engines" << std::endl;
    message << "========================================" << std::endl;
    for(auto&& name : availableInferenceEngines) {
        if(InferenceEngineManager::isEngineAvailable(name)) {
            message << name << std::endl;
            auto IE = InferenceEngineManager::loadEngine(name);
            try {
                auto deviceList = IE->getDeviceList();
                message << "Devices:" << std::endl;
                for (auto &&device : deviceList) {
                    message << device.index << " " << (int) device.type << " " << device.name << std::endl;
                }
            } catch(Exception &e) {

            }
        } else {
            message << name << " NOT AVAILABLE" << std::endl;
        }
    }

    // Check if test data is downloaded
    message << std::endl << "Test data" << std::endl;
    message << "========================================" << std::endl;
    Reporter::setGlobalReportMethod(Reporter::WARNING, Reporter::NONE);
    message << "Path: " << Config::getTestDataPath() << std::endl;
    if(!fileExists(join(Config::getTestDataPath(), "LICENSE.md"))) {
        message << "Dataset not downloaded. Download automatically by running the ./downloadTestDataIfNotExists executable" << std::endl;
    } else {
        message << "Dataset present" << std::endl;
    }
    Reporter::setGlobalReportMethod(Reporter::WARNING, Reporter::COUT);

    return message.str();
}

class SystemCheckWindow : public Window {
    FAST_OBJECT(SystemCheckWindow)
public:
    SystemCheckWindow() {
        setTitle("FAST " + getVersion() + " System Check");
        auto message = doCheck();

        auto importer = ImageImporter::New();
        importer->setFilename(Config::getDocumentationPath() + "/images/FAST_logo_square.png");
        importer->setGrayscale(false);

        // Set up GUI
        auto layout = new QVBoxLayout();
        setCenterLayout(layout);

        auto renderer = ImageRenderer::New();
        renderer->setInputConnection(importer->getOutputPort());

        auto view = createView();
        view->set2DMode();
        view->addRenderer(renderer);
        view->setFixedHeight((float)mHeight/2);
        layout->addWidget(view);

        auto textField = new QTextEdit;
        textField->setText(message.c_str());
        textField->setReadOnly(true);
        layout->addWidget(textField);
    }
};

int main(int argc, char**argv) {

    if(Config::getVisualization()) {
        auto window = SystemCheckWindow::New();
        window->start();
    } else {
        std::cout << doCheck() << std::endl;

        // Pipeline check
        auto importer = ImageImporter::New();
        importer->setFilename(Config::getDocumentationPath() + "/images/FAST_logo_square.png");
        importer->setGrayscale(false);
        auto image = importer->updateAndGetOutputData<Image>();
    }

    return 0;
}
