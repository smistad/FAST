set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/examples)
if(WIN32)
    file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.bat "")
else(WIN32)
    file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.sh "#!/bin/bash\n")
endif(WIN32)

### Macro for add examples
macro (fast_add_example NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} FAST)
    if(WIN32)
        file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.sh "examples\${NAME}.exe\n")
    else(WIN32)
        file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.sh "./examples/${NAME}\n")
    endif(WIN32)
endmacro()

add_subdirectory(DataImport)