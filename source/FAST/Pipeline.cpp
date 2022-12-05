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
#include <FAST/Visualization/View.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

namespace fast {

Pipeline::Pipeline(std::string filename, std::map<std::string, std::string> arguments) {
    mFilename = filename;

    // Read entire contents of file, replace arguments or throw error if not supplied
    // Store contents in buffer
    std::ifstream file(mFilename);
    if(!file.is_open())
        throw Exception("Unable to open file " + filename);

    do {
        std::string line;
        std::getline(file, line);
        line = replace(line, "$TEST_DATA_PATH$", Config::getTestDataPath());
        line = replace(line, "$CURRENT_PATH$", getDirName(filename));

        trim(line);
        if(line.empty() || line[0] == '#')
            continue;

        // Get name and description
        auto key = line.substr(0, line.find(' '));
        auto value = line.substr(line.find(' ') + 1);
        trim(value);
        if(key == "PipelineName") {
            value = replace(value, "\"", "");
            mName = value;
        } else if(key == "PipelineDescription") {
            value = replace(value, "\"", "");
            mDescription = value;
        } else if(key == "PipelineOutputData") {
            auto parts = split(value, " ");
            if(parts.size() != 3)
                throw Exception("PipelineOutputData must have 3 values");
            trim(parts[0]); // Unique name
            trim(parts[1]); // process object producing this output
            trim(parts[2]); // Port
            m_pipelineOutputData[parts[0]] = std::make_pair(parts[1], std::stoi(parts[2]));
        } else if(key == "PipelineInputData") {
            auto parts = split(value, " ");
            if(parts.empty())
                throw Exception("PipelineInputData must ate least 1 value");
            std::string name = parts[0];
            trim(name); // Unique name
            std::string description = "";
            try {
                value.substr(parts[0].size()+1);
                description = replace(description, "\"", "");
            } catch(...) {

            }
            m_pipelineInputData[name] = std::make_pair(description, nullptr);
        }

    // Check for variables @@
    std::size_t foundStart = line.find("@@");
        while(foundStart != std::string::npos) {
            auto foundEnd = line.find("@@", foundStart + 2);
            if (foundEnd == std::string::npos)
                throw Exception("Variable name was not closed with @@ in pipeline file");
            std::string variableName = line.substr(foundStart+2, foundEnd - foundStart - 2);
            const std::string toReplace = variableName;
            auto parts = split(variableName, "=");
            bool hasDefaultValue = false;
            if (parts.size() == 2) {
                variableName = parts[0];
                hasDefaultValue = true;
            }

            Reporter::info() << "Found variable " << variableName << " in pipeline file" << Reporter::end();
            // Replace variable
            if (arguments.count(variableName) == 0) {
                if (!hasDefaultValue)
                    throw Exception("The pipeline file requires you to give a value for the variable named " + variableName + "\n"
                        + "This is done by adding --" + variableName + " <value> to the command line arguments");
                arguments[variableName] = parts[1];
            }
            line = replace(line, "@@" + toReplace + "@@", arguments[variableName]);
            foundStart = line.find("@@", foundEnd + 2);
        }
        m_lines.push_back(line);

    } while (!file.eof());
}

inline std::shared_ptr<ProcessObject> getProcessObjectFromRegistry(std::string name) {
    return ProcessObjectRegistry::create(name);
}

void Pipeline::parseView(
        std::string objectID,
        int& lineNr
        ) {


    ProcessObject* view = m_views[objectID];

    // Continue to scan file for attributes
    ++lineNr;
    while(lineNr < m_lines.size()) {
        std::string line = m_lines[lineNr];
        trim(line);
        if(line == "")
            break;

        std::vector<std::string> tokens = split(line);
        if(tokens[0] != "Attribute")
            break;

        if(tokens.size() < 3)
            throw Exception("Expecting at least 3 items on attribute line when a view " + line);

        std::string name = tokens[1];

        std::shared_ptr<Attribute> attribute = view->getAttribute(name);
        std::string attributeValues = line.substr(line.find(name) + name.size());
        trim(attributeValues);
        attribute->parseInput(attributeValues);
        Reporter::info() << "Set attribute " << name << " to " << attributeValues  << " for object " << objectID << Reporter::end();
        ++lineNr;
    }

    view->loadAttributes();
}

void Pipeline::parseProcessObject(
        std::string objectName,
        std::string objectID,
        int& lineNr,
        bool isRenderer
    ) {

    // Create object
    std::shared_ptr<ProcessObject> object = getProcessObjectFromRegistry(objectName);

    std::string line = "";

    // Continue to scan file for attributes
    ++lineNr;
    while(lineNr < m_lines.size()) {
        std::string line = m_lines[lineNr];
        trim(line);
        if(line == "")
            break;

        std::vector<std::string> tokens = split(line);
        if(tokens[0] != "Attribute")
            break;

        if(tokens.size() < 3)
            throw Exception("Expecting at least 3 items on attribute line when parsing object " + objectName + " but got " + line);

        std::string name = tokens[1];

        if(name == "execute-on-last-frame-only") {
            object->setExecuteOnLastFrameOnly(true);
            Reporter::info() << "Set attribute " << name << " to true for object " << objectID << Reporter::end();
        } else {
            std::shared_ptr<Attribute> attribute = object->getAttribute(name);
            std::string attributeValues = line.substr(line.find(name) + name.size());
            trim(attributeValues);
            attribute->parseInput(attributeValues);
            Reporter::info() << "Set attribute " << name << " to " << attributeValues  << " for object " << objectID << Reporter::end();
        }
        ++lineNr;
    }
    --lineNr;

    object->loadAttributes();
    object->setModified(true);

    mProcessObjects[objectID] = object;
    Reporter::info() << "Added process object " << objectName  << " with id " << objectID << Reporter::end();

    // Get inputs and connect the POs and renderers
    ++lineNr;
    while (lineNr < m_lines.size()) {
        std::string line = m_lines[lineNr];
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
        if(tokens.size() == 3) {
            // Check if it is registered as pipeline input data
            if(m_pipelineInputData.count(inputID) > 0) {
                if(!m_pipelineInputData[inputID].second)
                    throw Exception("This pipeline requires input data named " + inputID + ", but no such data was given to Pipeline::parse()");
                if(isRenderer) {
                    Reporter::info() << "Connected data object " << inputID << " to renderer " << objectID << Reporter::end();
                    std::shared_ptr<Renderer> renderer = std::static_pointer_cast<Renderer>(object);
                    renderer->addInputData(m_pipelineInputData[inputID].second);
                } else {
                    Reporter::info() << "Connected data object " << inputID << " to " << objectID << Reporter::end();
                    object->connect(inputPortID, m_pipelineInputData[inputID].second);
                }
                ++lineNr;
                continue;
            }
        }
        int outputPortID = 0;
        if(tokens.size() == 4)
            outputPortID = std::stoi(tokens[3]);

        if(mProcessObjects.count(inputID) == 0)
            throw Exception("Input with id " + inputID + " was not found before " + objectID);

        if(isRenderer) {
            Reporter::info() << "Connected process object " << inputID << " to renderer " << objectID << Reporter::end();
            std::shared_ptr<Renderer> renderer = std::static_pointer_cast<Renderer>(object);
            renderer->addInputConnection(mProcessObjects.at(inputID)->getOutputPort(outputPortID));
        } else {
            Reporter::info() << "Connected process object " << inputID << " to " << objectID << Reporter::end();
            object->setInputConnection(inputPortID, mProcessObjects.at(inputID)->getOutputPort(outputPortID));
        }
        ++lineNr;
    }

    if(isRenderer) {
        mRenderers.push_back(objectID);
    }
}

void Pipeline::parse(std::map<std::string, std::shared_ptr<DataObject>> inputData, std::map<std::string, std::shared_ptr<ProcessObject>> processObjects, bool visualization) {
    // Parse file again, retrieve process objects, set attributes and create the pipeline

    for(auto item : inputData) {
        if(m_pipelineInputData.count(item.first) == 0) {
            Reporter::warning() << "Pipeline did not declaring needing any input data named " << item.first << ". Should be declared using PipelineInputData <name> <description>" << Reporter::end();
            m_pipelineInputData[item.first] = std::make_pair("", item.second);
        } else {
            m_pipelineInputData[item.first].second = item.second;
        }
    }
    mProcessObjects = processObjects;
    mRenderers.clear();
    m_views.clear();

    // Retrieve all POs and renderers
    for(int lineNr = 0; lineNr < m_lines.size(); ++lineNr) {
        std::string line = m_lines[lineNr];
        trim(line);

        if(line.empty()) {
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
            parseProcessObject(object, id, lineNr);
            lineNr--;
        } else if(key == "Renderer" && visualization) {
            if(tokens.size() != 3) {
                throw Exception("Unable to parse pipeline file " + mFilename + ", expected 3 tokens but got line " + line);
            }
            std::string id = tokens[1];
            std::string object = tokens[2];
            parseProcessObject(object, id, lineNr, true);
            lineNr--;
            Reporter::info() << "Added renderer " << object  << " with id " << id << Reporter::end();
        } else if(key == "View" && visualization) {
            // Create a view
            View *view = new View();
            std::string id = tokens[1];
            Reporter::info() << "Added view with id " << id << Reporter::end();
            m_views[id] = view;
            if(tokens.size() > 2) {
                for(int i = 2; i < tokens.size(); ++i) {
                    if(mProcessObjects.count(tokens[i]) == 0)
                        throw Exception("Renderer with name " + tokens[i] + " not found in pipeline file.");

                    view->addRenderer(std::dynamic_pointer_cast<Renderer>(mProcessObjects[tokens[i]]));
                    Reporter::info() << "Added renderer " << tokens[i] << " to the view" << Reporter::end();
                }
            } else {
                // View has no renderers.. throw error message?
            }
            parseView(id, lineNr);
            lineNr--;
        } else if(key == "Attribute") {
            // Custom field/attribute/metadata..
            std::string value = line.substr(line.find(tokens[1]) + tokens[1].size());
            trim(value);
            value = replace(value, "\"", "");
            Reporter::info() << "Found pipeline attribute " << tokens[1] << " in pipeline file with value " << value << Reporter::end();
            m_attributes[tokens[1]] = value;
        }
    }
    m_parsed = true;
}

std::vector<View*> Pipeline::getViews() const {
    Reporter::info() << "Setting up pipeline.." << Reporter::end();
    if(mProcessObjects.empty())
        throw Exception("You have to parse the pipeline file before calling getViews on the pipeline");

    // Get renderers
    std::vector<View*> views;
    for(auto&& view : m_views) {
        views.push_back(view.second);
    }

    Reporter::info() << "Finished setting up pipeline." << Reporter::end();

    return views;
}

std::vector<std::shared_ptr<Renderer>> Pipeline::getRenderers() {
    std::vector<std::shared_ptr<Renderer>> result;
    for(auto&& rendererName : mRenderers)
        result.push_back(std::dynamic_pointer_cast<Renderer>(mProcessObjects[rendererName]));

    return result;
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

std::vector<Pipeline> getAvailablePipelines(std::string path) {
    std::vector<Pipeline> pipelines;
    if(path.empty())
        path = Config::getPipelinePath();
    if(!QDir(path.c_str()).exists())
        throw Exception("Pipeline path " + path + " does not exist");
    // List all files in this directory ending with .fpl
    QDirIterator it(path.c_str(), QStringList() << "*.fpl", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
		std::string filepath = it.next().toUtf8().constData();

        try {
            Pipeline pipeline(filepath);
            pipelines.push_back(pipeline);
        } catch(Exception & e) {
            Reporter::warning() << "Error reading pipeline " << filepath << ": " << e.what() << Reporter::end();
            continue;
        }
    }
    return pipelines;
}

std::map<std::string, std::shared_ptr<ProcessObject>> Pipeline::getProcessObjects() const {
    if(mProcessObjects.empty())
        throw Exception("You have to parse the pipeline before getting process objects");

    return mProcessObjects;
}

std::string Pipeline::getPipelineAttribute(std::string key) const {
    if(m_attributes.count(key) == 0)
        throw Exception("Attribute " + key + " was not found in the pipeline text file");
    return m_attributes.at(key);
}

DataObject::pointer Pipeline::getPipelineOutputData(std::string name, std::function<void(float)> progressFunction) {
    if(mProcessObjects.empty())
        throw Exception("You have to parse the pipeline before getting output data");
    if(m_pipelineOutputData.count(name) == 0)
        throw Exception("Pipeline output data " + name + " not found.");

    auto [POname, port] = m_pipelineOutputData[name];
    auto PO = getProcessObject(POname);
    auto data = PO->runAndGetOutputData(port);

    // Check if data is "in progress"
    if(data->hasFrameData("progress")) {
        do {
            // If so, we update until it is marked as finished
            data = PO->runAndGetOutputData(port);
            if(progressFunction != nullptr) {
                // Report progress
                progressFunction(std::stof(data->getFrameData("progress")));
            }
        } while(!data->isLastFrame());
    }

    return data;
}

std::map<std::string, DataObject::pointer> Pipeline::getAllPipelineOutputData(std::function<void(float)> progressFunction) {
    if(mProcessObjects.empty())
        throw Exception("You have to parse the pipeline before getting output data");
    if(m_pipelineOutputData.empty())
        return {};

    std::map<std::string, DataObject::pointer> result;
    int64_t executeToken = 0;
    for(auto [name, output] : m_pipelineOutputData) {
        auto PO = getProcessObject(output.first);
        result[name] = PO->runAndGetOutputData(output.second, executeToken);
        // Check if data is marked as "in progress"
        if(result[name]->hasFrameData("progress")) {
            do {
                // If so, we update until it is marked as finished
                executeToken++;
                result[name] = PO->runAndGetOutputData(output.second, executeToken);
                if(progressFunction != nullptr) {
                    // Report progress
                    progressFunction(std::stof(result[name]->getFrameData("progress")));
                }
            } while(!result[name]->isLastFrame());
        }
    }

    return result;
}

std::shared_ptr<ProcessObject> Pipeline::getProcessObject(std::string name) {
    if(mProcessObjects.count(name) == 0)
        throw Exception("Process object " + name + "not found in pipeline");
    auto PO = mProcessObjects[name];
    return PO;
}

std::map<std::string, std::string> Pipeline::getRequiredPipelineInputData() const {
    std::map<std::string, std::string> inputs;
    for(auto item : m_pipelineInputData)
        inputs[item.first] = item.second.first;
    return inputs;
}

std::map<std::string, std::string> Pipeline::getPipelineAttributes() const {
    return m_attributes;
}

bool Pipeline::isParsed() const {
    return m_parsed;
}

Pipeline Pipeline::fromDataHub(std::string itemID, std::map<std::string, std::string> variables, DataHub&& hub) {
    auto result = hub.download(itemID);
    return Pipeline(join(hub.getStorageDirectory(), itemID, "pipeline.fpl"), variables);
}

std::map<std::string, DataObject::pointer> Pipeline::run(std::map<std::string, std::shared_ptr<DataObject>> inputData, std::map<std::string, std::shared_ptr<ProcessObject>> processObjects, bool visualization) {
    if(!isParsed())
        parse(inputData, processObjects, visualization);

    if(!m_views.empty()) {
        auto window = MultiViewWindow::create(0);
        for(auto&& view : getViews())
            window->addView(view);
        window->run(); // Visualize and block here
        return getAllPipelineOutputData();
    } else {
        return getAllPipelineOutputData();
    }
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

ProcessObjectWidget::ProcessObjectWidget(std::shared_ptr<ProcessObject> po, QWidget *parent) : QWidget(parent) {
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