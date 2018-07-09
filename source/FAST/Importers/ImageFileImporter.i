// This adds support for converting std::string into python strings
%include "std_string.i"

%include "FAST/ProcessObject.i"

%shared_ptr(fast::ImageFileImporter)

namespace fast {

class ImageFileImporter : public ProcessObject {
    public:
    	static std::shared_ptr<ImageFileImporter> New();
        void setFilename(std::string filename);
	private:
    	ImageFileImporter();
};

%template(ImageFileImporterPtr) std::shared_ptr<ImageFileImporter>;

} // end namespace fast
