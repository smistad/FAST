#ifndef ImageExporter_HPP_
#define ImageExporter_HPP_

#include "ProcessObject.hpp"
#include "Image.hpp"
#include <string>

namespace fast {

class ImageExporter : public ProcessObject {
    FAST_OBJECT(ImageExporter)
    public:
        void setInput(ImageData::pointer image);
        void setFilename(std::string filename);
    private:
        ImageExporter();
        void execute();

        std::string mFilename;
        ImageData::pointer mInput;

};


} // end namespace fast




#endif /* ImageExporter_HPP_ */
