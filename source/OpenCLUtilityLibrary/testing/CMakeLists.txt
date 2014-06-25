
set(OpenCLUtilityLibrary_INCLUDE_DIRS
    ${OpenCLUtilityLibrary_INCLUDE_DIRS}
    ./catch
)

include_directories(
    ${OpenCLUtilityLibrary_INCLUDE_DIRS}
    )

set(SOURCE_FILES
    TestFixture.hpp
    TestFixture.cpp
)

set(EXECUTABLE_SOURCE_FILES
    catch/catch.hpp
    CatchMain.cpp
    histogramPyramidTests.cpp
    tests.cpp
)

set(OpenCLUtilityLibrary_LINK_LIBRARIES
    ${OpenCLUtilityLibrary}
)

add_library (TestOpenCLUtilityLibrary ${SOURCE_FILES})
target_link_libraries(TestOpenCLUtilityLibrary OpenCLUtilityLibrary ${OpenCLUtilityLibrary_LINK_LIBRARIES})

set(TestOpenCLUtilityLibrary_LIBRARY
    TestOpenCLUtilityLibrary
    PARENT_SCOPE
   )

add_executable (Catch ${EXECUTABLE_SOURCE_FILES} ${SOURCE_FILES})
target_link_libraries(Catch OpenCLUtilityLibrary ${OpenCLUtilityLibrary_LINK_LIBRARIES})

