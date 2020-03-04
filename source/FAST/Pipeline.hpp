#pragma once

#include <string>
#include <vector>
#include <QToolBox>
#include <unordered_map>
#include <unordered_set>
#include <FAST/ProcessObject.hpp>

namespace fast {

class Renderer;
class View;

class FAST_EXPORT  Pipeline : public Object {
    public:
        Pipeline(std::string name, std::string description, std::string filename);
        std::vector<View*> setup();
        std::unordered_map<std::string, SharedPointer<ProcessObject>> getProcessObjects();
        std::string getName() const;
        std::string getDescription() const;
        std::string getFilename() const;
        /**
         * Parse the pipeline file
         */
        void parsePipelineFile();

    private:
        std::string mName;
        std::string mDescription;
        std::string mFilename;
        std::unordered_map<std::string, SharedPointer<ProcessObject>> mProcessObjects;
        std::unordered_map<std::string, View*> m_views;
        std::vector<std::string> mRenderers;

        void parseProcessObject(
            std::string objectName,
            std::string objectID,
            std::ifstream& file,
            bool isRenderer = false
        );
        void parseView(
                std::string objectID,
                std::ifstream& file
        );
};

/**
 * Retrieve a list of all pipelines stored in .fpl files in the specified pipeline directory
 * @return
 */
FAST_EXPORT std::vector<Pipeline> getAvailablePipelines();

class FAST_EXPORT  PipelineWidget : public QToolBox {
    public:
        PipelineWidget(Pipeline pipeline, QWidget* parent = nullptr);

};

class FAST_EXPORT  ProcessObjectWidget : public QWidget {
    public:
        ProcessObjectWidget(SharedPointer<ProcessObject> po, QWidget* parent = nullptr);
};

} // end namespace fast

