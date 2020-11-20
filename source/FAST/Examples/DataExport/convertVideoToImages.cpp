#include <FAST/Streamers/MovieStreamer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>
#include <FAST/Exporters/ImageFileExporter.hpp>
#include <thread>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Convert video to images",
        "This example will convert all movies in the given path to a set of images."
        "If --export-to is not set, the images will be displayed. If it is set the images will be exported to this path."
    );
    parser.addPositionVariable(1, "path", true, "Either path to directory or movie file. If directiory it will convert all movies in this path. Example /some/path/");
    parser.addPositionVariable(2, "extension", false, "If path is dir, will convert all movies with this extension. Example: mp4");
    parser.addVariable("export-to", false, "If this is set, each frame of the video is stored as an image on disk in this directory.");
    parser.addOption("ultrasound-cropping", "Enable ultrasound cropping. Useful for videos exported from ultrasound scanners.");
    parser.addVariable("physical-width", false, "Set the physical with (in mm) of the image to calculate the pixel spacing.");
    parser.addChoice("export-format", {"mhd", "png", "bmp", "jpg"}, "mhd", "Select image format to export");
    parser.addOption("static-cropping", "Enable static ultrasound cropping. Meaning that the cropping parameters are calculated for the first frame and then used for the rest");
    parser.addOption("disable-compression", "Disable compression when saving as mhd (.zraw)");
    parser.addOption("color", "Use color");
    parser.parse(argc, argv);

    std::string path = parser.get("path");
    std::vector<std::string> files;
    if(isDir(path)) {
		std::string extension = parser.get("extension");
        for(auto file : getDirectoryList(parser.get("path"))) {
            if(file.size() < extension.size())
                continue;
            if(file.substr(file.size() - extension.size()) != extension)
                continue;
            files.push_back(join(path, file));
        }
    } else {
        files.push_back(path);
    }

    for(auto file : files) {
        file = replace(file, "\\", "/");
        const std::string extension = file.substr(file.rfind('.'));
        Reporter::info() << "Processing file " << file << Reporter::end();
        auto streamer = MovieStreamer::New();
        streamer->setGrayscale(!parser.getOption("color"));
        streamer->setFilename(file);

        auto port = streamer->getOutputPort();
        if(parser.getOption("ultrasound-cropping")) {
            auto cropper = UltrasoundImageCropper::New();
            cropper->setInputConnection(port);
            cropper->setStaticCropping(parser.getOption("static-cropping"));
            if(parser.gotValue("physical-width"))
                cropper->setPhysicalWidth(parser.get<int>("physical-width"));
            port = cropper->getOutputPort();
        }

        if(parser.gotValue("export-to")) {
            std::string exportPath = parser.get("export-to");
            exportPath += file.substr(file.rfind('/') + 1, file.size() - extension.size() - 1 - file.rfind('/')) + "/";
            createDirectories(exportPath);
            int timestep = 0;
            auto exporter = ImageFileExporter::New();
            exporter->setCompression(!parser.getOption("disable-compression"));
            bool stop = false;
            while(!stop) {
                streamer->update();
                auto image = port->getNextFrame<Image>();
				exporter->setInputData(image);
                stop = image->isLastFrame();
                std::cout << "Processing frame: " << timestep << std::endl;
                const std::string path = exportPath + "frame_" + std::to_string(timestep) + "." + parser.get("export-format");
                exporter->setFilename(path);
                exporter->update();
                ++timestep;
            }
        } else {
            auto renderer = ImageRenderer::New();
            renderer->addInputConnection(port);

            auto window = SimpleWindow::New();
            window->addRenderer(renderer);
            window->getView()->setAutoUpdateCamera(true);
            window->getView()->setBackgroundColor(Color(0.1, 0.1, 0.1));
            window->set2DMode();
            window->start();
        }
    }
}