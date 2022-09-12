fast_add_sources(
    MetaImageExporter.cpp
    MetaImageExporter.hpp
    VTKMeshFileExporter.cpp
    VTKMeshFileExporter.hpp
    FileExporter.cpp
    FileExporter.hpp
    ImageFileExporter.cpp
    ImageFileExporter.hpp
    StreamToFileExporter.cpp
    StreamToFileExporter.hpp
    Exporter.hpp
)
fast_add_python_interfaces(Exporter.hpp FileExporter.hpp)
fast_add_python_shared_pointers(Exporter FileExporter)
fast_add_process_object(MetaImageExporter MetaImageExporter.hpp)
fast_add_process_object(ImageFileExporter ImageFileExporter.hpp)
fast_add_process_object(StreamToFileExporter StreamToFileExporter.hpp)
fast_add_process_object(VTKMeshFileExporter VTKMeshFileExporter.hpp)
fast_add_test_sources(
    Tests/MetaImageExporterTests.cpp
    Tests/VTKMeshFileExporterTests.cpp
)
if(FAST_MODULE_Visualization)
    fast_add_sources(
        ImageExporter.cpp
        ImageExporter.hpp
    )
    fast_add_process_object(ImageExporter ImageExporter.hpp)
    fast_add_test_sources(
        Tests/ImageExporterTests.cpp
    )
endif()
if(FAST_MODULE_VTK)
    fast_add_sources(
        VTKImageExporter.cpp
        VTKImageExporter.hpp
        VTKMeshExporter.cpp
        VTKMeshExporter.hpp
    )
    fast_add_test_sources(
        Tests/VTKImageExporterTests.cpp
        Tests/VTKMeshExporterTests.cpp
    )
endif()
if(FAST_MODULE_ITK)
    fast_add_sources(
        ITKImageExporter.hpp
    )
    fast_add_test_sources(
        Tests/ITKImageExporterTests.cpp
    )
endif()
if(FAST_MODULE_HDF5)
    fast_add_sources(
        HDF5TensorExporter.hpp
        HDF5TensorExporter.cpp
    )
    fast_add_process_object(HDF5TensorExporter HDF5TensorExporter.hpp)
    fast_add_test_sources(Tests/HDF5TensorExporterTests.cpp)
endif()
if(FAST_MODULE_WholeSlideImaging)
    fast_add_sources(
        ImagePyramidPatchExporter.cpp
        ImagePyramidPatchExporter.hpp
        TIFFImagePyramidExporter.cpp
        TIFFImagePyramidExporter.hpp
    )
    fast_add_test_sources(Tests/TIFFImagePyramidExporterTests.cpp Tests/ImagePyramidPatchExporterTests.cpp)
    fast_add_process_object(ImagePyramidPatchExporter ImagePyramidPatchExporter.hpp)
    fast_add_process_object(TIFFImagePyramidExporter TIFFImagePyramidExporter.hpp)
endif()
