#include "FileExporter.hpp"

namespace fast {

void FileExporter::setFilename(std::string filename) {
#ifdef WIN32
    m_filename = replace(filename, "\\", "/");
#else
    m_filename = filename;
#endif
    setModified(true);
}

FileExporter::FileExporter() {
    m_filename = "";
    createStringAttribute("filename", "Filename", "Filename to export to", "");
}

void FileExporter::loadAttributes() {
    setFilename(getStringAttribute("filename")) ;
}

}