fast_add_python_interfaces(DATA MeshVertex.hpp) # Must be before Mesh access objects
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
    Transform.cpp
    Transform.hpp
)
fast_add_test_sources(
    Tests/DataObjectTests.cpp
    Tests/ImageTests.cpp
)
fast_add_process_object(DATA BoundingBoxSetAccumulator BoundingBox.hpp)
fast_add_python_interfaces(DATA Image.hpp Mesh.hpp TensorShape.hpp Tensor.hpp Text.hpp MeshVertex.hpp Transform.hpp SimpleDataObject.hpp)
fast_add_python_shared_pointers(DATA Image BoundingBox BoundingBoxSet Mesh Tensor Segmentation Text)

if(FAST_MODULE_WholeSlideImaging)
    fast_add_sources(ImagePyramid.cpp ImagePyramid.hpp)
    fast_add_python_interfaces(DATA ImagePyramid.hpp)
    fast_add_python_shared_pointers(DATA ImagePyramid)
    fast_add_test_sources(Tests/ImagePyramidTests.cpp)
endif()


