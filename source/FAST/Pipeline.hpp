#ifndef FAST_PIPELINE_HPP_
#define FAST_PIPELINE_HPP_

#include <string>
#include <vector>
#include <QToolBox>
#include <unordered_map>
#include <unordered_set>
#include <FAST/SmartPointers.hpp>
#include <FAST/ProcessObject.hpp>

namespace fast {

class Renderer;

class FAST_EXPORT  Pipeline {
    public:
        Pipeline(std::string name, std::string description, std::string filename);
        std::vector<SharedPointer<Renderer> > setup(ProcessObjectPort input);
        std::unordered_map<std::string, SharedPointer<ProcessObject>> getProcessObjects();
        std::string getName() const;
        std::string getDescription() const;
        std::string getFilename() const;

    private:
        std::string mName;
        std::string mDescription;
        std::string mFilename;
        std::unordered_map<std::string, SharedPointer<ProcessObject>> mProcessObjects;
        /**
         * Names of which POs need input and the port id
         */
        std::unordered_map<std::string, uint> mInputProcessObjects;
        std::vector<std::string> mRenderers;

        void parsePipelineFile();
        void parseProcessObject(
            std::string objectName,
            std::string objectID,
            std::ifstream& file,
            bool isRenderer = false
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

#endif
