#ifndef ImageImporter_HPP_
#define ImageImporter_HPP_

#include "Importer.hpp"
#include <string>

namespace fast {

class FAST_EXPORT  ImageImporter : public Importer {
    FAST_OBJECT(ImageImporter)
    public:
        void setFilename(std::string filename);
        void setGrayscale(bool grayscale);
        ~ImageImporter() {};
    private:
        ImageImporter();
        std::string mFilename;
        bool mGrayscale;
        void execute();

};


} // end namespace fast



#endif /* ImageImporter_HPP_ */
