#### Add all subdirs
fast_add_subdirectories(Algorithms Data Streamers Importers Exporters Visualization Tests)
fast_add_sources(
    Object.cpp
    Object.hpp
    SmartPointers.hpp
    ProcessObject.cpp
    ProcessObject.hpp
    ExecutionDevice.cpp
    ExecutionDevice.hpp
    DeviceManager.cpp
    DeviceManager.hpp
    Exception.cpp
    Exception.hpp
    Utility.cpp
    Utility.hpp
    SceneGraph.cpp
    SceneGraph.hpp
    AffineTransformation.cpp
    AffineTransformation.hpp
)
fast_add_test_sources()