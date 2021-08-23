if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2021.1
                69628ba6f40094834b0f8b6f38efee6e7fc67cfb44805ffd638177de571aff3e
        )
    else()
        fast_download_dependency(openvino
                2021.1
                821a32723ed6427242a8130b7cda53a0cd44616c29a28dbc7e4835c9725d476d
        )
    endif()
endif()
