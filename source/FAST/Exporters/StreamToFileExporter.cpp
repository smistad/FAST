#include <FAST/Data/Image.hpp>
#include <FAST/Data/Mesh.hpp>
#include "StreamToFileExporter.hpp"
#include "ImageFileExporter.hpp"
#include "VTKMeshFileExporter.hpp"
#include "MetaImageExporter.hpp"
#include <FAST/Utility.hpp>

namespace fast {


void StreamToFileExporter::setPath(std::string path) {
    m_path = path;
}

void StreamToFileExporter::setRecordingFolderName(std::string folder) {
    m_folder = folder;
    m_currentFolder = folder;
}

void StreamToFileExporter::setFrameFilename(std::string name) {
    m_filename = name;
}

void StreamToFileExporter::setEnabled(bool enabled) {
    m_enabled = enabled;
}

void StreamToFileExporter::setFrameLimit(uint64_t limit) {
    m_frameLimit = limit;
}

uint64_t StreamToFileExporter::getFrameCounter() const {
    return m_frameCounter;
}

void StreamToFileExporter::execute() {
    // Get data object
    auto input = getInputData<DataObject>();
    if(m_path.empty())
        throw Exception("You must give a path to StreamToFileExporter");
    if(!m_enabled) {
        addOutputData(0, input);
        return;
    }
    if(!m_hasStarted) {
        m_currentFolder = m_folder;
        // Use timestamp to create folder if one is not already set
        if(m_folder.empty()) {
            m_currentFolder = currentDateTime();
        }
        // Create directory
        createDirectories(join(m_path, m_currentFolder));
        m_hasStarted = true;
        m_recordingStartTime = std::chrono::high_resolution_clock::now();
    }

    if(m_frameCounter >= m_frameLimit)
        throw Exception("Maximum nr of frames (" + std::to_string(m_frameLimit) + ") reached in StreamToFileExporter");

    std::string currentFileName = join(m_path, m_currentFolder, m_filename + "_" + std::to_string(m_frameCounter));
    if(auto imageInput = std::dynamic_pointer_cast<Image>(input)) {
        auto exporter = MetaImageExporter::New();
        exporter->enableCompression();
        exporter->setFilename(currentFileName + ".mhd");
        exporter->setInputData(input);
        exporter->update();
    } else if(auto meshInput = std::dynamic_pointer_cast<Mesh>(input)) {
        auto exporter = VTKMeshFileExporter::New();
        exporter->setFilename(currentFileName + ".vtk");
        exporter->setInputData(input);
        exporter->update();
    } else {
        throw Exception("StreamToFileExporter can only handle Image and Mesh data objects");
    }
    m_frameCounter += 1;
    addOutputData(0, input);
}

void StreamToFileExporter::reset() {
    m_frameCounter = 0;
    m_currentFolder = "";
    m_hasStarted = false;
}

StreamToFileExporter::StreamToFileExporter() {
    createInputPort<DataObject>(0);
    createOutputPort<DataObject>(0);
}

bool StreamToFileExporter::isEnabled() {
    return m_enabled;
}

std::string StreamToFileExporter::getCurrentDestinationFolder() const {
    if(m_currentFolder.empty() && (!m_enabled && !m_hasStarted))
        throw Exception("Can't get current destination folder until started in StreamToFileExporter");

    return join(m_path, m_currentFolder);
}

float StreamToFileExporter::getRecordingDuration() const {
    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - m_recordingStartTime;
    return (float)duration.count();
}

}