%{
#define SWIG_FILE_WITH_INIT
%}
%include "FAST/SmartPointers.i"
%shared_ptr(fast::Object)
%shared_ptr(fast::DataObject)
%shared_ptr(fast::SpatialDataObject)
%shared_ptr(fast::Mesh)

%include "std_vector.i"

%template(vectorf) std::vector<float>;

// Returns a list of floats consisting of all vertex positions and normals
%inline %{
std::vector<float> fast_get_vertex_data(fast::SharedPointer<fast::Mesh> mesh) {
    fast::MeshAccess::pointer access = mesh->getMeshAccess(ACCESS_READ);
    std::vector<fast::MeshVertex> vertices = access->getVertices();
    std::vector<float> result;
    for(fast::MeshVertex v : vertices) {
        result.push_back(v.getPosition().x());
        result.push_back(v.getPosition().y());
        if(mesh->getDimensions() > 2) {
            result.push_back(v.getPosition().z());
        }
        result.push_back(v.getNormal().x());
        result.push_back(v.getNormal().y());
        if(mesh->getDimensions() > 2) {
            result.push_back(v.getNormal().z());
        }
    }
    return result;
}
%}

namespace fast {

%ignore Object;
class Object {
};
%ignore DataObject;
class DataObject : public Object {
};
%ignore SpatialDataObject;
class SpatialDataObject : public DataObject {
};


class Mesh : public SpatialDataObject {
    FAST_OBJECT(Mesh)
    public:
        unsigned int getNrOfTriangles() const;
        unsigned int getNrOfLines() const;
        unsigned int getNrOfVertices() const;
        uchar getDimensions() const;
    protected:
        Mesh();
};

%template(MeshPtr) SharedPointer<Mesh>;

}