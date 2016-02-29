// This adds support for converting std::string into python strings
%include "std_string.i"

%import "FAST/ProcessObject.i"

namespace fast {


class ImageFileImporter : public ProcessObject {
    public:
    	ImageFileImporter();
        void setFilename(std::string filename);
	private:
		void execute(); // Must have this, or class will be defined as abstract 
};

} // end namespace fast