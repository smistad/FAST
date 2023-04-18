if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2022.3.0
                6f652dc6c533d2894650db728a0c2bc6e60e817ff6343f1dc30756b6ccadad0c
        )
    elseif(APPLE)
        fast_download_dependency(openvino
                2022.3.0
                69f11b1cd008db87d972346a2ff4036c7c4d8371c2192f661911b357c8eb4fe4
	)
    else()
        fast_download_dependency(openvino
                2022.3.0
                405b1e4156b96877606501a10402b992d04b54431dbf81605e435bcbd9d9f2bd
        )

    endif()
endif()
