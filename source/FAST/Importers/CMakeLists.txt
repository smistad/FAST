fast_add_sources(
    VTKMeshFileImporter.cpp
    VTKMeshFileImporter.hpp
    MetaImageImporter.cpp
    MetaImageImporter.hpp
    Importer.hpp
    NIFTIImporter.cpp
    NIFTIImporter.hpp
    ImageFileImporter.cpp
    ImageFileImporter.hpp
)
# The order here is important, this must come first:
fast_add_python_interfaces(Importer.hpp FileImporter.hpp)
fast_add_python_shared_pointers(Importer FileImporter)

fast_add_process_object(ImageFileImporter ImageFileImporter.hpp)
fast_add_process_object(MetaImageImporter MetaImageImporter.hpp)
fast_add_process_object(ImageImporter ImageImporter.hpp)
fast_add_process_object(VTKMeshFileImporter VTKMeshFileImporter.hpp)
fast_add_process_object(NIFTIImporter NIFTIImporter.hpp)
fast_add_test_sources(
    Tests/MetaImageImporterTests.cpp
    Tests/VTKMeshFileImporterTests.cpp
    Tests/ImageFileImporterTests.cpp
    Tests/NIFTIImporterTests.cpp
)
if(FAST_MODULE_Dicom)
fast_add_sources(
    DICOMFileImporter.cpp
    DICOMFileImporter.hpp
)
fast_add_process_object(DICOMFileImporter DICOMFileImporter.hpp)
fast_add_test_sources(
    Tests/DICOMFileImporterTests.cpp
)
endif()
if(FAST_MODULE_Visualization)
    fast_add_sources(
        ImageImporter.cpp
        ImageImporter.hpp
    )
    fast_add_test_sources(
        Tests/ImageImporterTests.cpp
    )
endif()
if(FAST_MODULE_VTK)
    fast_add_sources(
        VTKImageImporter.cpp
        VTKImageImporter.hpp
    )
    fast_add_test_sources(
        Tests/VTKImageImporterTests.cpp
    )
endif()
if(FAST_MODULE_ITK)
    fast_add_sources(
        ITKImageImporter.hpp
    )
    fast_add_test_sources(
        Tests/ITKImageImporterTests.cpp
    )
endif()

if(FAST_MODULE_WholeSlideImaging)
    fast_add_sources(
        WholeSlideImageImporter.cpp
        WholeSlideImageImporter.hpp
        ImagePyramidPatchImporter.cpp
        ImagePyramidPatchImporter.hpp
        TIFFImagePyramidImporter.cpp
        TIFFImagePyramidImporter.hpp
    )
    fast_add_test_sources(
        Tests/WholeSlideImageImporterTests.cpp
    )
    fast_add_process_object(WholeSlideImageImporter WholeSlideImageImporter.hpp)
    fast_add_process_object(ImagePyramidPatchImporter ImagePyramidPatchImporter.hpp)
    fast_add_process_object(TIFFImagePyramidImporter TIFFImagePyramidImporter.hpp)
endif()
if(FAST_MODULE_HDF5)
    fast_add_sources(
        HDF5TensorImporter.cpp
        HDF5TensorImporter.hpp
    )
    fast_add_process_object(HDF5TensorImporter HDF5TensorImporter.hpp)
    fast_add_test_sources(Tests/HDF5TensorImporterTests.cpp)
endif()