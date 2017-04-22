#include "Pipeline.hpp"
#include "FAST/Config.hpp"
#include "ProcessObject.hpp"
#include <QDirIterator>
#include <fstream>
#include "ProcessObjectList.hpp"

namespace fast {

Pipeline::Pipeline(std::string name, std::string description, std::string filename) {
    mName = name;
    mDescription = description;
    mFilename = filename;
}

inline SharedPointer<ProcessObject> getProcessObject(std::string name) {
    return ProcessObjectRegistry::create(name);
}

inline SharedPointer<ProcessObject> parseProcessObject(
        std::string objectName,
        std::string objectID,
        std::ifstream& file,
        const std::unordered_map<std::string, SharedPointer<ProcessObject>>& processObjects,
        const ProcessObjectPort& pipelineInput,
        bool isRenderer = false
    ) {

    // Create object
    SharedPointer<ProcessObject> object = getProcessObject(objectName);

    std::string line = "";
    std::getline(file, line);

    // Get inputs
    bool inputFound = false;
    while(!file.eof()) {
        trim(line);
        if(line == "")
            break;

        std::vector<std::string> tokens = split(line);
        if(tokens[0] != "Input")
            break;
        if(tokens.size() < 3)
            throw Exception("Expecting at least 3 items on input line when parsing object " + objectName + " but got " + line);

        int inputPortID = std::stoi(tokens[1]);
        std::string inputID = tokens[2];
        int outputPortID = 0;
        if(tokens.size() == 4)
            outputPortID = std::stoi(tokens[3]);

        if(inputID != "PipelineInput" && processObjects.count(inputID) == 0)
            throw Exception("Input with id " + inputID + " was not found before " + objectID);

        inputFound = true;

        if(isRenderer) {
            SharedPointer<Renderer> renderer = object;
            // TODO fix text renderer no supprt addInput
            if(inputID == "PipelineInput") {
                //renderer->addInputConnection(pipelineInput);
                renderer->setInputConnection(0, pipelineInput);
            } else {
                //renderer->addInputConnection(processObjects.at(inputID)->getOutputPort(outputPortID));
                renderer->setInputConnection(0, processObjects.at(inputID)->getOutputPort(outputPortID));
            }
        } else {
            if(inputID == "PipelineInput") {
                object->setInputConnection(inputPortID, pipelineInput);
            } else {
                object->setInputConnection(inputPortID, processObjects.at(inputID)->getOutputPort(outputPortID));
            }
        }
        std::getline(file, line);
    }

    if(!inputFound)
        throw Exception("No inputs were found for process object " + objectName);

    std::cout << "attributes.." << std::endl;
    // Continue to scan file for attributes
    while(!file.eof()) {
        trim(line);
        if(line == "")
            break;

        std::vector<std::string> tokens = split(line);
        if(tokens[0] != "Attribute")
            throw Exception("Expecting attribute when parsing object " + objectName + " but got line " + line);

        if(tokens.size() < 3)
            throw Exception("Expecting at least 3 items on attribute line when parsing object " + objectName + " but got " + line);

        std::string name = tokens[1];

        std::shared_ptr<Attribute> attribute = object->getAttribute(name);
        std::string attributeValues = line.substr(line.find(name) + name.size());
        trim(attributeValues);
        std::cout << "parse input.." << std::endl;
        attribute->parseInput(attributeValues);
        std::getline(file, line);
    }


    return object;
}

std::vector<SharedPointer<Renderer>> Pipeline::setup(ProcessObjectPort input) {
    std::cout << "setting up pipeline.." << std::endl;
    // Parse file again, retrieve process objects, set attributes and create the pipeline
    std::ifstream file(mFilename);
    std::string line = "";
    std::getline(file, line);

    std::unordered_map<std::string, SharedPointer<ProcessObject> > processObjects;
    std::vector<SharedPointer<Renderer> > renderers;

    // Retrieve all POs and renderers
    while(!file.eof()) {
        trim(line);

        if(line.size() == 0) {
            std::getline(file, line);
            continue;
        }

        std::vector<std::string> tokens = split(line);

        std::string key = tokens[0];

        if(key == "ProcessObject") {
            if(tokens.size() != 3) {
                throw Exception("Unable to parse pipeline file " + mFilename + ", expected 3 tokens but got line " + line);
            }
            std::string id = tokens[1];
            std::string object = tokens[2];
            processObjects[id] = parseProcessObject(object, id, file, processObjects, input);
        } else if(key == "Renderer") {
            if(tokens.size() != 3) {
                throw Exception("Unable to parse pipeline file " + mFilename + ", expected 3 tokens but got line " + line);
            }
            std::string id = tokens[1];
            std::string object = tokens[2];
            renderers.push_back(parseProcessObject(object, id, file, processObjects, input, true));
        }

        std::getline(file, line);
    }

    if(renderers.size() == 0)
        throw Exception("No renderers were found when parsing pipeline file " + mFilename);

    // For each PO, load attributes
    for(auto processObject : processObjects) {
        processObject.second->loadAttributes();
    }

    // For each renderer, load attributes
    for(auto renderer : renderers) {
        renderer->loadAttributes();
    }
    std::cout << "finished" << std::endl;

    return renderers;
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