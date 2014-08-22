#ifndef METAIMAGEEXPORTER_HPP_
#define METAIMAGEEXPORTER_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include <string>

namespace fast {

class MetaImageExporter : public ProcessObject {
    FAST_OBJECT(MetaImageExporter)
    public:
        void setInput(ImageData::pointer input);
        void setFilename(std::string filename);
    private:
        MetaImageExporter();
        void execute();

        std::string mFilename;
        ImageData::pointer mInput;
};

} // end namespace fast



#endif /* METAIMAGEEXPORTER_HPP_ */
