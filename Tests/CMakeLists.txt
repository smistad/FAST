
set(SOURCE_FILES
#    TestFixture.hpp
#    TestFixture.cpp
)

set(EXECUTABLE_SOURCE_FILES
    catch.hpp
    CatchMain.cpp
)

set(FAST_LINK_LIBRARIES
    ${FAST}
)

#add_library (TestFAST ${SOURCE_FILES})
#target_link_libraries(TestFAST FAST ${FAST_LINK_LIBRARIES})

#set(TestFAST_LIBRARY
#    TestFAST
#    PARENT_SCOPE
#)

add_executable (test ${EXECUTABLE_SOURCE_FILES} ${SOURCE_FILES})
target_link_libraries(test FAST ${FAST_LINK_LIBRARIES})

