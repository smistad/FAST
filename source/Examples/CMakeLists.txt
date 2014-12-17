set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/examples)
if(WIN32)
    file(WRITE ${EXECUTABLE_OUTPUT_PATH}/runAllExamples.bat "")
else(WIN32)
    file(WRITE ${EXECUTABLE_OUTPUT_PATH}/runAllExamples.sh "#!/bin/bash\n")
endif(WIN32)

### Macro for add examples
macro (fast_add_example NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} FAST)
    if(WIN32)
        file(APPEND ${EXECUTABLE_OUTPUT_PATH}/runAllExamples.sh "${NAME}.exe\n")
    else(WIN32)
        file(APPEND ${EXECUTABLE_OUTPUT_PATH}/runAllExamples.sh "./${NAME}\n")
    endif(WIN32)
endmacro()

add_subdirectory(DataImport)