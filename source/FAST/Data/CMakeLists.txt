fast_add_python_interfaces(MeshVertex.hpp) # Must be before Mesh access objects
fast_add_subdirectories(Access)
fast_add_sources(
    BoundingBox.cpp
    BoundingBox.hpp
    DataBoundingBox.cpp
    DataBoundingBox.hpp
    DataObject.cpp
    DataObject.hpp
    SpatialDataObject.cpp
    SpatialDataObject.hpp
    Image.cpp
    Image.hpp
    Segmentation.cpp
    Segmentation.hpp
    DataTypes.cpp
    DataTypes.hpp
    Mesh.cpp
    Mesh.hpp
    MeshVertex.cpp
    MeshVertex.hpp
    Color.hpp
    Camera.cpp
    Camera.hpp
    SimpleDataObject.hpp
    Tensor.cpp
    Tensor.hpp
    TensorShape.cpp
    TensorShape.hpp
    Text.cpp
    Text.hpp
)
fast_add_test_sources(
    Tests/DataObjectTests.cpp
    Tests/ImageTests.cpp
)
fast_add_process_object(BoundingBoxSetAccumulator BoundingBox.hpp)
fast_add_python_interfaces(Image.hpp Mesh.hpp)
fast_add_python_shared_pointers(Image BoundingBox BoundingBoxSet Mesh)

if(FAST_MODULE_WholeSlideImaging)
    fast_add_sources(ImagePyramid.cpp ImagePyramid.hpp)
endif()


