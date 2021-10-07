include(cmake/Externals.cmake)
#### Macro for adding source files and directories
macro (fast_add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_SOURCE_FILES "${_relPath}/${_src}")
        else()
            list (APPEND FAST_SOURCE_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate FAST_SOURCE_FILES to parent directory
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
    endif()
endmacro()

macro (fast_add_test_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_TEST_SOURCE_FILES "${_relPath}/${_src}")
        else()
            list (APPEND FAST_TEST_SOURCE_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate FAST_TEST_SOURCE_FILES to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
    endif()
endmacro()

macro(fast_add_python_interfaces)
    file(RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/source/" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach(_src ${ARGN})
        if(_relPath)
            list(APPEND FAST_PYTHON_HEADER_FILES "${_relPath}/${_src}")
        else()
            list(APPEND FAST_PYTHON_HEADER_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate to parent directory
        set(FAST_PYTHON_HEADER_FILES ${FAST_PYTHON_HEADER_FILES} PARENT_SCOPE)
    endif()
endmacro()

macro(fast_add_python_shared_pointers)
    foreach(_shared_ptr_object ${ARGN})
        list(APPEND FAST_PYTHON_SHARED_PTR_OBJECTS ${_shared_ptr_object})
    endforeach()
        # propagate to parent directory
        set(FAST_PYTHON_SHARED_PTR_OBJECTS ${FAST_PYTHON_SHARED_PTR_OBJECTS} PARENT_SCOPE)
endmacro()

macro (fast_add_subdirectories)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        add_subdirectory(${_src})
    endforeach()
    if (_relPath)
        # propagate to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_EXAMPLES ${FAST_EXAMPLES} PARENT_SCOPE)
        set (FAST_PROCESS_OBJECT_NAMES ${FAST_PROCESS_OBJECT_NAMES} PARENT_SCOPE)
        set (FAST_PROCESS_OBJECT_HEADER_FILES ${FAST_PROCESS_OBJECT_HEADER_FILES} PARENT_SCOPE)
        set (FAST_INFERENCE_ENGINES ${FAST_INFERENCE_ENGINES} PARENT_SCOPE)
        set (FAST_PYTHON_HEADER_FILES ${FAST_PYTHON_HEADER_FILES} PARENT_SCOPE)
        set (FAST_PYTHON_SHARED_PTR_OBJECTS ${FAST_PYTHON_SHARED_PTR_OBJECTS} PARENT_SCOPE)
    endif()
endmacro()

macro (fast_add_all_subdirectories)
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
            add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${child})
        endif()
    endforeach()
    if (_relPath)
        # propagate to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_EXAMPLES ${FAST_EXAMPLES} PARENT_SCOPE)
        set (FAST_PROCESS_OBJECT_NAMES ${FAST_PROCESS_OBJECT_NAMES} PARENT_SCOPE)
        set (FAST_PROCESS_OBJECT_HEADER_FILES ${FAST_PROCESS_OBJECT_HEADER_FILES} PARENT_SCOPE)
        set (FAST_INFERENCE_ENGINES ${FAST_INFERENCE_ENGINES} PARENT_SCOPE)
        set (FAST_PYTHON_HEADER_FILES ${FAST_PYTHON_HEADER_FILES} PARENT_SCOPE)
        set (FAST_PYTHON_SHARED_PTR_OBJECTS ${FAST_PYTHON_SHARED_PTR_OBJECTS} PARENT_SCOPE)
    endif()
endmacro()

### Macro for add examples
macro (fast_add_example NAME)
    if(FAST_BUILD_EXAMPLES)
        list(APPEND FAST_EXAMPLES ${NAME})
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} FAST)
        install(TARGETS ${NAME}
            DESTINATION fast/bin
            COMPONENT fast
        )
        if(WIN32)
            file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.bat "bin\\${NAME}.exe\r\n")
        else()
            file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.sh "./bin/${NAME}\n")
        endif()
        file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
        if(_relPath)
            # propagate to parent directory
            set(FAST_EXAMPLES ${FAST_EXAMPLES} PARENT_SCOPE)
        endif()
    endif()
endmacro()

### Macro for add tool
macro (fast_add_tool NAME)
    if(FAST_BUILD_TOOLS)
        list(APPEND FAST_TOOLS ${NAME})
        add_executable(${NAME} ${ARGN})
        target_link_libraries(${NAME} FAST)
        install(TARGETS ${NAME}
                DESTINATION fast/bin
                COMPONENT fast
        )
        file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
        if(_relPath)
            # propagate to parent directory
            set(FAST_TOOLS ${FAST_TOOLS} PARENT_SCOPE)
        endif()
    endif()
endmacro()

### Macro for add process objects
macro(fast_add_process_object NAME HEADERFILE)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/source/" "${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND FAST_PROCESS_OBJECT_NAMES ${NAME})
    list(APPEND FAST_PYTHON_SHARED_PTR_OBJECTS ${NAME})
    if(_relPath)
        list(APPEND FAST_PROCESS_OBJECT_HEADER_FILES ${_relPath}/${HEADERFILE})
        list(APPEND FAST_PYTHON_HEADER_FILES ${_relPath}/${HEADERFILE})
    endif()
    if(_relPath)
        # propagate to parent directory
        set(FAST_PROCESS_OBJECT_NAMES ${FAST_PROCESS_OBJECT_NAMES} PARENT_SCOPE)
        set(FAST_PROCESS_OBJECT_HEADER_FILES ${FAST_PROCESS_OBJECT_HEADER_FILES} PARENT_SCOPE)
        set(FAST_PYTHON_HEADER_FILES ${FAST_PYTHON_HEADER_FILES} PARENT_SCOPE)
        set(FAST_PYTHON_SHARED_PTR_OBJECTS ${FAST_PYTHON_SHARED_PTR_OBJECTS} PARENT_SCOPE)
    endif()
endmacro()

### Macro for add inference engines
macro(fast_add_inference_engine NAME)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/source/" "${CMAKE_CURRENT_SOURCE_DIR}")

    # FILES are additional arguments to this macro
    set(FILES ${ARGN})

    list(APPEND FAST_INFERENCE_ENGINES InferenceEngine${NAME})
    if(_relPath)
        # propagate to parent directory
        set(FAST_INFERENCE_ENGINES ${FAST_INFERENCE_ENGINES} PARENT_SCOPE)
    endif()
endmacro()

# Macro for downloading a dependency
macro(fast_download_dependency NAME VERSION SHA)
    set(FILENAME ${NAME}_${VERSION}_${FAST_DEPENDENCY_TOOLSET}.tar.xz)
    # TODO correct spelling mistake: licences
    if(WIN32)
    if(${NAME} STREQUAL qt5)
        ExternalProject_Add(${NAME}
                PREFIX ${FAST_EXTERNAL_BUILD_DIR}/${NAME}
                URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
                URL_HASH SHA256=${SHA}
                UPDATE_COMMAND ""
                CONFIGURE_COMMAND ""
                BUILD_COMMAND ""
                # On install: Copy contents of each subfolder to the build folder
                INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
                    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${FAST_EXTERNAL_INSTALL_DIR}/bin COMMAND
                    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
                    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/plugins ${FAST_EXTERNAL_INSTALL_DIR}/plugins COMMAND
		    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licenses | echo COMMAND
		    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licenses ${FAST_EXTERNAL_INSTALL_DIR}/licenses | echo
        )
    else()
        ExternalProject_Add(${NAME}
                PREFIX ${FAST_EXTERNAL_BUILD_DIR}/${NAME}
                URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
                URL_HASH SHA256=${SHA}
                UPDATE_COMMAND ""
                CONFIGURE_COMMAND ""
                BUILD_COMMAND ""
                # On install: Copy contents of each subfolder to the build folder
                INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
                    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${FAST_EXTERNAL_INSTALL_DIR}/bin COMMAND
                    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
		    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licenses | echo COMMAND
		    ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licenses ${FAST_EXTERNAL_INSTALL_DIR}/licenses | echo
                )
    endif()
    else(WIN32)
        # copy_directory doesn't support symlinks, use cp on linux/apple:
        ExternalProject_Add(${NAME}
                PREFIX ${FAST_EXTERNAL_BUILD_DIR}/${NAME}
                URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
                URL_HASH SHA256=${SHA}
                UPDATE_COMMAND ""
                CONFIGURE_COMMAND ""
                BUILD_COMMAND ""
                # On install: Copy contents of each subfolder to the build folder
                INSTALL_COMMAND cp -r <SOURCE_DIR>/include/. ${FAST_EXTERNAL_INSTALL_DIR}/include/ COMMAND
                cp -r <SOURCE_DIR>/bin/. ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
                cp -a <SOURCE_DIR>/lib/. ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
                cp -a <SOURCE_DIR>/plugins/. ${FAST_EXTERNAL_INSTALL_DIR}/plugins/ | echo "" COMMAND
                cp -r <SOURCE_DIR>/licences/. ${FAST_EXTERNAL_INSTALL_DIR}/licenses/ | echo "" COMMAND
                cp -r <SOURCE_DIR>/licenses/. ${FAST_EXTERNAL_INSTALL_DIR}/licenses/ | echo ""
                )
    endif()
    foreach(LIB ${ARGN})
        list(APPEND LIBRARIES ${ARGN})
    endforeach()
    list(APPEND FAST_EXTERNAL_DEPENDENCIES ${NAME})
endmacro()
