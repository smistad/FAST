set(SOURCE_FILES
#    TestFixture.hpp
#    TestFixture.cpp
)

set(EXECUTABLE_SOURCE_FILES
    catch.hpp
    CatchMain.cpp
    DummyObjects.hpp
    ProcessObjectTests.cpp
    DataObjectTests.cpp
    Algorithms/GaussianSmoothingFilterTests.cpp
    Data/ImageTests.cpp
    Importers/ImageImporterTests.cpp
    Exporters/ImageExporterTests.cpp
    Importers/MetaImageImporterTests.cpp
    Exporters/MetaImageExporterTests.cpp
)

include_directories(
    ./
)

if(VTK_INTEROP)
set(EXECUTABLE_SOURCE_FILES
    ${EXECUTABLE_SOURCE_FILES} 
    Importers/VTKImageImporterTests.cpp
    Exporters/VTKImageExporterTests.cpp
)
endif()

if(ITK_INTEROP)
set(EXECUTABLE_SOURCE_FILES
    ${EXECUTABLE_SOURCE_FILES} 
    Importers/ITKImageImporterTests.cpp
    Exporters/ITKImageExporterTests.cpp
)
endif()

set(FAST_LINK_LIBRARIES
    ${FAST}
)

#add_library (TestFAST ${SOURCE_FILES})
#target_link_libraries(TestFAST FAST ${FAST_LINK_LIBRARIES})

#set(TestFAST_LIBRARY
#    TestFAST
#    PARENT_SCOPE
#)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFAST_TEST_DATA_DIR=\\\"${PROJECT_SOURCE_DIR}/Tests/TestData/\\\"")

add_executable (test ${EXECUTABLE_SOURCE_FILES} ${SOURCE_FILES})
target_link_libraries(test FAST ${FAST_LINK_LIBRARIES})

