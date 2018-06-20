#include <FAST/Streamers/MovieStreamer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>
#include <FAST/Exporters/ImageFileExporter.hpp>
#include <thread>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Convert video to images",
        "This example will convert all movies in the given path to a set of images."
        "If --export-to is not set, the images will be displayed. If it is set the images will be exported to this path."
    );
    parser.addPositionVariable(1, "path", true, "Will convert all movies in this path. Example /some/path/");
    parser.addPositionVariable(2, "extension", true, "Will convert all movies with this extension. Example: mp4");
    parser.addVariable("export-to", false, "If this is set, each frame of the video is stored as an image on disk in this directory.");
    parser.addOption("ultrasound-cropping", "Enable ultrasound cropping. Useful for videos exported from ultrasound scanners.");
    parser.addVariable("physical-width", false, "Set the physical with (in mm) of the image to calculate the pixel spacing.");
    parser.addChoice("export-format", {"mhd", "png", "bmp", "jpg"}, "mhd", "Select image format to export");
    parser.parse(argc, argv);

    std::string path = parser.get("path");
    std::string extension = parser.get("extension");
    for(auto file : getDirectoryList(parser.get("path"))) {
        std::cout << file << std::endl;
        if(file.substr(file.size() - extension.size()) != extension)
            continue;
        MovieStreamer::pointer streamer = MovieStreamer::New();
        streamer->setFilename(path + file);

        auto port = streamer->getOutputPort();
        if(parser.getOption("ultrasound-cropping")) {
            UltrasoundImageCropper::pointer cropper = UltrasoundImageCropper::New();
            cropper->setInputConnection(port);
            if(parser.gotValue("physical-width"))
                cropper->setPhysicalWidth(parser.get<int>("physical-width"));
            port = cropper->getOutputPort();
        }

        if(parser.gotValue("export-to")) {
            std::string exportPath = parser.get("export-to");
            exportPath += file.substr(0, file.size() - extension.size() - 1) + "/";
            createDirectories(exportPath);
            int timestep = 0;
            ImageFileExporter::pointer exporter = ImageFileExporter::New();
            exporter->setInputConnection(port);
            bool stop = false;
            while(true) {
                std::cout << "Processing frame: " << timestep << std::endl;
                const std::string path = exportPath + "frame_" + std::to_string(timestep) + "." + parser.get("export-format");
                exporter->setFilename(path);
                exporter->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
                ++timestep;

                // TODO fix overcomplicated stop process
                while(streamer->getFramesAdded() - 1 < timestep) {
                    std::cout << "Waiting for frames" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if(streamer->hasReachedEnd()) {
                        stop = true;
                        break;
                    }
                }
                if(stop)
                    break;
            }
        } else {
            ImageRenderer::pointer renderer = ImageRenderer::New();
            renderer->addInputConnection(port);

            SimpleWindow::pointer window = SimpleWindow::New();
            window->addRenderer(renderer);
            window->getView()->setAutoUpdateCamera(true);
            window->getView()->setBackgroundColor(Color(0.1, 0.1, 0.1));
            window->set2DMode();
            window->start();
        }
    }
}