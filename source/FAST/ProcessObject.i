%include "FAST/SmartPointers.i"
%include "FAST/Data/Image.i"

%shared_ptr(fast::Object)
%shared_ptr(fast::ProcessObject)

%inline %{
fast::std::shared_ptr<fast::Image> getNextFrameImage(fast::std::shared_ptr<fast::ProcessObject> po) {
    fast::DynamicData::pointer dd = po->getOutputData<fast::Image>();
    fast::Image::pointer image = dd->getNextFrame(po);
    return image;
}
fast::std::shared_ptr<fast::Mesh> getNextFrameMesh(fast::std::shared_ptr<fast::ProcessObject> po) {
    fast::DynamicData::pointer dd = po->getOutputData<fast::Mesh>();
    fast::Mesh::pointer mesh = dd->getNextFrame(po);
    return mesh;
}
%}

typedef unsigned int uint;
typedef unsigned char uchar;
namespace fast {

#define FAST_OBJECT(name) public:\
static std::shared_ptr<name> New();\
private:\
name();\

class ProcessObjectPort {
};

class Object {
};

class ProcessObject : public Object {
    public:
    	void update();
        ProcessObjectPort getOutputPort();
        void setInputConnection(ProcessObjectPort port);
        void setInputConnection(uint connectionID, ProcessObjectPort port);
        template <class T>
        std::shared_ptr<T> getOutputData();
        template <class T>
        std::shared_ptr<T> getOutputData(int port);
        %template(getStaticImageOutputData) getOutputData<Image>;

        template <class T>
        std::shared_ptr<DataObject> getOutputData();
        %template(getImageOutputData) getOutputData<Image>;
    protected:
    	ProcessObject();
    	virtual void execute() = 0;
};




} // end namespace fast