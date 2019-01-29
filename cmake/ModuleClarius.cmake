if(FAST_MODULE_Clarius)
    option(CLARIUS_SDK_DIR "Set" "")
    message("-- Clarius ultrasound module enabled. Clarius SDK dir set to: ${CLARIUS_SDK_DIR}")

    list(APPEND FAST_INCLUDE_DIRS ${CLARIUS_SDK_DIR}/include/)
    list(APPEND LIBRARIES ${CLARIUS_SDK_DIR}/lib/liblisten.so)
endif()