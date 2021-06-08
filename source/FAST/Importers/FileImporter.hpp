#pragma once

#include <FAST/Importers/Importer.hpp>

namespace fast {
/**
 * @brief Abstract base class for importers which import a file with a filename.
 *
 * @ingroup importers
 */
class FAST_EXPORT FileImporter : public Importer {
    public:
        virtual void setFilename(std::string filename) {
            m_filename = filename;
            setModified(true);
        }
        std::string getFilename() const {
            return m_filename;
        }
        void loadAttributes() {
            setFilename(getStringAttribute("filename"));
        }
    protected:
        FileImporter(std::string filename) : FileImporter() {
            setFilename(filename);
        }
        FileImporter() {
            createStringAttribute("filename", "Filename", "Filename of the file to import", "");
        }
        virtual void execute() = 0;

        std::string m_filename;

};

}