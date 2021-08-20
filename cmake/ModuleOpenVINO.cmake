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
                821a32723ed6427242a8130b7cda53a0cd44616c29a28dbc7e4835c9725d476d
        )
    endif()
endif()
