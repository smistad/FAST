#include "Pipeline.hpp"
#include "FAST/Config.hpp"
#include "ProcessObject.hpp"
#include <QDirIterator>
#include <fstream>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
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

void Pipeline::parseProcessObject(
        std::string objectName,
        std::string objectID,
        std::ifstream& file,
        bool isRenderer
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

        if(inputID != "PipelineInput" && mProcessObjects.count(inputID) == 0)
            throw Exception("Input with id " + inputID + " was not found before " + objectID);

        inputFound = true;


        if(isRenderer) {
            SharedPointer<Renderer> renderer = object;
            // TODO fix text renderer no supprt addInput
            if(inputID == "PipelineInput") {
                mInputProcessObjects[objectID] = 0;
            } else {
                //renderer->addInputConnection(processObjects.at(inputID)->getOutputPort(outputPortID));
                renderer->setInputConnection(0, mProcessObjects.at(inputID)->getOutputPort(outputPortID));
            }
        } else {
            if(inputID == "PipelineInput") {
                mInputProcessObjects[objectID] = inputPortID;
            } else {
                object->setInputConnection(inputPortID, mProcessObjects.at(inputID)->getOutputPort(outputPortID));
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

    mProcessObjects[objectID] = object;
    if(isRenderer) {
        mRenderers.push_back(objectID);
    }
}

void Pipeline::parsePipelineFile() {
    // Parse file again, retrieve process objects, set attributes and create the pipeline
    std::ifstream file(mFilename);
    std::string line = "";
    std::getline(file, line);

    mProcessObjects.clear();
    mInputProcessObjects.clear();
    mRenderers.clear();

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
            parseProcessObject(object, id, file);
        } else if(key == "Renderer") {
            if(tokens.size() != 3) {
                throw Exception("Unable to parse pipeline file " + mFilename + ", expected 3 tokens but got line " + line);
            }
            std::string id = tokens[1];
            std::string object = tokens[2];
            parseProcessObject(object, id, file, true);
        }

        std::getline(file, line);
    }

    if(mRenderers.size() == 0)
        throw Exception("No renderers were found when parsing pipeline file " + mFilename);

    // For each PO, load attributes
    for(auto processObject : mProcessObjects) {
        processObject.second->loadAttributes();
    }

    std::cout << "finished" << std::endl;
}
std::vector<SharedPointer<Renderer>> Pipeline::setup(ProcessObjectPort input) {
    std::cout << "setting up pipeline.." << std::endl;
    parsePipelineFile();

    // Set input process object port to all needed
    for(std::pair<std::string, uint> inputPort : mInputProcessObjects) {
        mProcessObjects[inputPort.first]->setInputConnection(inputPort.second, input);
    }

    // Get renderers
    std::vector<SharedPointer<Renderer>> renderers;
    for(auto renderer : mRenderers) {
        renderers.push_back(mProcessObjects[renderer]);
    }

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
    if(!QDir(path.c_str()).exists())
        throw Exception("Pipeline path " + path + " does not exist");
    // List all files in this directory ending with .fpl
    QDirIterator it(path.c_str(), QStringList() << "*.fpl", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
		std::string filename = it.next().toUtf8().constData();
		std::cout << filename << std::endl;
        std::ifstream file(filename);
		if (!file.is_open()) {
			throw Exception("Unable to open file " + filename);
		}

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
	std::cout << "asd" << std::endl;
    return pipelines;
}

std::unordered_map<std::string, SharedPointer<ProcessObject>> Pipeline::getProcessObjects() {
    if(mProcessObjects.size() == 0)
        parsePipelineFile();

    return mProcessObjects;
}


PipelineWidget::PipelineWidget(Pipeline pipeline, QWidget* parent) : QToolBox(parent) {
    auto processObjects = pipeline.getProcessObjects();
    for(auto object : processObjects) {
        ProcessObjectWidget* widget = new ProcessObjectWidget(object.second, this);
        addItem(widget, (object.first + " - " + object.second->getNameOfClass()).c_str());
    }
    setCurrentIndex(processObjects.size()-1);

    setStyleSheet(
            "QToolBox::tab {\n"
            "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
            "                                stop: 0 #689fb6, stop: 1.0 #6d93a7);\n"
            "    border: 1px solid #004d5b;\n"
            "    border-radius: 2px;\n"
            "    color: white;\n"
            "}\n"
            "\n"
            "QToolBox::tab:selected { \n"
            "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
            "                                stop: 0 #2e8eb6, stop: 1.0 #4084a7);\n"
            "}"
    );
}

ProcessObjectWidget::ProcessObjectWidget(SharedPointer<ProcessObject> po, QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    auto attributes = po->getAttributes();
    for(auto attr : attributes) {
        std::string id = attr.first;
        std::shared_ptr<Attribute> attribute = attr.second;

        QLabel* label = new QLabel(this);
        label->setText(attribute->getName().c_str());
        layout->addWidget(label);

        if(attribute->getType() == ATTRIBUTE_TYPE_STRING) {
            QLineEdit *textBox = new QLineEdit(this);
            std::shared_ptr<AttributeValueString> stringAttribute = std::dynamic_pointer_cast<AttributeValueString>(
                    attribute->getValue());
            textBox->setText(stringAttribute->get().c_str());
            layout->addWidget(textBox);
        } else if(attribute->getType() == ATTRIBUTE_TYPE_FLOAT) {
            QLineEdit *textBox = new QLineEdit(this);
            std::shared_ptr<AttributeValueFloat> stringAttribute = std::dynamic_pointer_cast<AttributeValueFloat>(
                    attribute->getValue());
            textBox->setText(std::to_string(stringAttribute->get()).c_str());
            layout->addWidget(textBox);
        } else if(attribute->getType() == ATTRIBUTE_TYPE_INTEGER) {
            QLineEdit *textBox = new QLineEdit(this);
            std::shared_ptr<AttributeValueInteger> stringAttribute = std::dynamic_pointer_cast<AttributeValueInteger>(
                    attribute->getValue());
            textBox->setText(std::to_string(stringAttribute->get()).c_str());
            layout->addWidget(textBox);
        } else if(attribute->getType() == ATTRIBUTE_TYPE_BOOLEAN) {
            QCheckBox *checkBox = new QCheckBox(this);
            std::shared_ptr<AttributeValueBoolean> stringAttribute = std::dynamic_pointer_cast<AttributeValueBoolean>(
                    attribute->getValue());
            checkBox->setChecked(stringAttribute->get());
            layout->addWidget(checkBox);
        }
    }
    setLayout(layout);
}

}