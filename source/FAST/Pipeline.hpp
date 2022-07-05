#pragma once

#include <string>
#include <vector>
#include <QToolBox>
#include <unordered_map>
#include <unordered_set>
#include <FAST/ProcessObject.hpp>
#include <FAST/DataHub.hpp>

namespace fast {

class Renderer;
class View;

using StringMap = std::map<std::string, std::string>;
using DataMap = std::map<std::string, std::shared_ptr<DataObject>>;
using ProcessObjectMap = std::map<std::string, std::shared_ptr<ProcessObject>>;

/**
 * @brief A class representing a text pipeline.
 */
class FAST_EXPORT  Pipeline {
    public:
        /**
         * @brief Setup a text pipeline
         * @param filename path to text pipeline file
         * @param variables
         */
        Pipeline(std::string filename, StringMap variables = StringMap());
        /**
         * @brief Create pipeline object from DataHub
         * This will download the pipeline item from the DataHub and any dependencies (models, data etc.).
         *
         * @param itemID DataHub item ID
         * @param variables
         * @param hub
         * @return pipeline
         */
        static Pipeline fromDataHub(std::string itemID, StringMap variables = StringMap(), DataHub&& hub = DataHub());
        /**
         * @brief Get all views in this pipeline
         * @return list of views
         */
        std::vector<View*> getViews() const;
        /**
         * @brief Get renderers in this pipeline
         * @return list of renderers
         */
        std::vector<std::shared_ptr<Renderer>> getRenderers();
        /**
         * @brief Get all process objects in this pipeline, excluding renderers.
         * @return list of process objects
         */
        std::map<std::string, std::shared_ptr<ProcessObject>> getProcessObjects() const;
        /**
         * @brief Get a process object in this pipeline
         * @param name Name of process object to extract
         * @return process object
         */
        std::shared_ptr<ProcessObject> getProcessObject(std::string name);
        std::string getName() const;
        std::string getDescription() const;
        std::string getFilename() const;
        /**
         * @brief Get pipeline attribute
         * @param attribute name
         * @return attribute value
         */
        std::string getPipelineAttribute(std::string name) const;
        /**
         * @brief Get all pipeline attributes;
         * @return map of attributes
         */
        std::map<std::string, std::string> getPipelineAttributes() const;
        /**
         * @brief Parse the pipeline file
         *
         * @param inputData Input data objects
         * @param processObjects Process objects to connect to this pipeline
         * @param visualization If false parse will ignore any renderers and views
         */
        void parse(
                DataMap inputData = DataMap(),
                ProcessObjectMap processObjects = ProcessObjectMap(),
                bool visualization = true
        );

        /**
         * @brief Check if pipeline has been parsed
         * @return
         */
        bool isParsed() const;

        /**
         * @brief Parse and run this pipeline. If pipeline contains views, it will open a window and render.
         *
         * @param inputData Input data objects
         * @param processObjects Process objects to connect to this pipeline
         * @param visualization If false parse will ignore any renderers and views
         * @return pipeline output data
         */
        std::map<std::string, DataObject::pointer> run(
                DataMap inputData = DataMap(),
                ProcessObjectMap processObjects = ProcessObjectMap(),
                bool visualization = true
        );

        /**
         * @brief Get list of all required input data for this pipeline
         * @return
         */
        std::map<std::string, std::string> getRequiredPipelineInputData() const;
        /**
         * @brief Get all data objects which has been declared as output data in this pipeline
         *
         * Pipeline output data is declared using "PipelineOutputData <name> <process object> <output port id>"
         * This function will block until done. Use progress function to log progress.
         *
         * @param progressFunction
         * @return map of pipeline output data
         */
        std::map<std::string, DataObject::pointer> getAllPipelineOutputData(std::function<void(float)> progressFunction = nullptr);
        /**
          * @brief Get data object which has been declared as output data in this pipeline with a given name.
          *
          * Pipeline output data is declared using "PipelineOutputData <name> <process object> <output port id>"
          * This function will block until done. Use progress function to log progress.
          *
          * @param progressFunction
          * @return pipeline output data object
          */
        DataObject::pointer getPipelineOutputData(std::string name, std::function<void(float)> progressFunction = nullptr);
    private:
        bool m_parsed = false;
        std::string mName;
        std::string mDescription;
        std::string mFilename;
        std::map<std::string, std::shared_ptr<ProcessObject>> mProcessObjects;
        std::unordered_map<std::string, View*> m_views;
        std::vector<std::string> mRenderers;
        std::vector<std::string> m_lines;
        std::map<std::string, std::string> m_attributes;
        std::map<std::string, std::pair<std::string, uint>> m_pipelineOutputData;
        std::map<std::string, std::pair<std::string, std::shared_ptr<DataObject>>> m_pipelineInputData;

        void parseProcessObject(
            std::string objectName,
            std::string objectID,
            int& lineNr,
            bool isRenderer = false
        );
        void parseView(
                std::string objectID,
                int& lineNr
        );
};

/**
 * Retrieve a list of all pipelines stored in .fpl files in the specified pipeline directory
 * @return
 */
FAST_EXPORT std::vector<Pipeline> getAvailablePipelines(std::string path = "");

#ifndef SWIG
class FAST_EXPORT  PipelineWidget : public QToolBox {
    public:
        PipelineWidget(Pipeline pipeline, QWidget* parent = nullptr);

};

class FAST_EXPORT  ProcessObjectWidget : public QWidget {
    public:
        ProcessObjectWidget(std::shared_ptr<ProcessObject> po, QWidget* parent = nullptr);
};
#endif

} // end namespace fast

