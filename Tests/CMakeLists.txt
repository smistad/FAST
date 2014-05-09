set(SOURCE_FILES
#    TestFixture.hpp
#    TestFixture.cpp
)

set(EXECUTABLE_SOURCE_FILES
    catch.hpp
    CatchMain.cpp
    Data/ImageTests.cpp
    Importers/ImageImporterTests.cpp
    Importers/MetaImageImporterTests.cpp
)

include_directories(
    ./
)

if(VTK_INTEROP)
set(EXECUTABLE_SOURCE_FILES
    ${EXECUTABLE_SOURCE_FILES} 
    VTKInteropTests.cpp
)
endif()

if(ITK_INTEROP)
set(EXECUTABLE_SOURCE_FILES
    ${EXECUTABLE_SOURCE_FILES} 
    ITKInteropTests.cpp
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

