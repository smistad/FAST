# This will automatically download the FAST test data zip file and unzip it
# The data will only be downloaded if not already present
if(${FAST_DOWNLOAD_TEST_DATA})
    if(FAST_TEST_DATA_DIR STREQUAL "")
        set(TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/data/")
        set(FAST_TEST_DATA_DIR ${TEST_DATA_DIR})
    else()
        set(TEST_DATA_DIR "${FAST_TEST_DATA_DIR}")
    endif()
    if(NOT EXISTS ${TEST_DATA_DIR})
        message("-- Test data folder does not exists (${TEST_DATA_DIR}). Downloading data ...")
        set(TEST_DATA_FILENAME ${PROJECT_SOURCE_DIR}/FAST_Test_Data.zip)
        # Download
        file(DOWNLOAD
                "http://www.idi.ntnu.no/~smistad/FAST_Test_Data.zip"
                ${TEST_DATA_FILENAME}
                SHOW_PROGRESS
                STATUS DOWNLOAD_STATUS
                INACTIVITY_TIMEOUT 30
        )
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 STATUS_DESC)
        if(NOT ${STATUS_CODE} EQUAL 0)
            message("-- Download of test data failed: ${STATUS_DESC}")
            # TODO Try other download URL
        else()
            # Unzip
            execute_process(
                    COMMAND ${CMAKE_COMMAND}
                    -E tar xzf ${TEST_DATA_FILENAME} /data/ ${TEST_DATA_DIR}
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
            # Delete zip
            file(REMOVE ${TEST_DATA_FILENAME})
        endif()
    else()
        message("-- Test data folder exists (${TEST_DATA_DIR}). Skipping data download.")
    endif()
endif()
