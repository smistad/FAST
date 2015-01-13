#ifndef METAIMAGEEXPORTER_HPP_
#define METAIMAGEEXPORTER_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include <string>

namespace fast {

class MetaImageExporter : public ProcessObject {
    FAST_OBJECT(MetaImageExporter)
    public:
        void setFilename(std::string filename);
    private:
        MetaImageExporter();
        void execute();

        std::string mFilename;
};

} // end namespace fast



#endif /* METAIMAGEEXPORTER_HPP_ */
