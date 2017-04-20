#include "Pipeline.hpp"
#include "FAST/Config.hpp"
#include "ProcessObject.hpp"
#include <QDirIterator>
#include <fstream>

namespace fast {

Pipeline::Pipeline(std::string name, std::string description, std::string filename) {
    mName = name;
    mDescription = description;
    mFilename = filename;
}

std::vector<SharedPointer<Renderer>> Pipeline::setup(ProcessObjectPort input) {
    // TODO parse file
}

std::string Pipeline::getName() const {
    return mName;
}

std::string Pipeline::getDescription() const {
    return mDescription;
}

std::string Pipeline::getFilename() const {
    return mFilename;
}

std::vector<Pipeline> getAvailablePipelines() {
    std::vector<Pipeline> pipelines;
    std::string path = Config::getPipelinePath();
    // List all files in this directory ending with .fpl
    QDirIterator it(path.c_str(), QStringList() << "*.fpl", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        std::string filename = it.next().toStdString();
        std::ifstream file(filename);

        std::string name = "";
        std::string description = "";
        std::string line = "";
        std::getline(file, line);
        while(!file.eof()) {
            trim(line);
            int spacePos = line.find(" ");
            std::string key = line.substr(0, spacePos);
            if(key == "PipelineName") {
                name = line.substr(spacePos + 1);
                name = replace(name, "\"", " ");
                trim(name);
            } else if(key == "PipelineDescription") {
                description = line.substr(spacePos + 1);
                description = replace(description, "\"", " ");
                trim(description);
            }
            if(name.size() > 0 && description.size() > 0) {
                pipelines.push_back(Pipeline(name, description, filename));
                break;
            }
            std::getline(file, line);
        }
        if(name.size() == 0 || description.size() == 0) {
            throw Exception("Pipeline name and description not found in file " + filename);
        }

        file.close();
    }
    return pipelines;
}

}