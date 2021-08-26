if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2021.1
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
        )
    else()
        fast_download_dependency(openvino
                2021.1
                70c5c352748dabf85f119027c281fb37a968421ef6c7cf47de8fd520acb41b99
        )
    endif()
endif()
