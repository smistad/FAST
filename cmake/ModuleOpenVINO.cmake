if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2022.3.0
                d481dfa6ec31543121acf2ab23b701ce5c5b76da14ddebda9386580c686d2049
        )
    elseif(APPLE)
        fast_download_dependency(openvino
                2022.3.0
                c1ae81053fa2f2cbcead4619ccd28f56bde4a1d25880302b878d02fe859b1daf
	)
    else()
        fast_download_dependency(openvino
                2022.3.0
                405b1e4156b96877606501a10402b992d04b54431dbf81605e435bcbd9d9f2bd
        )

    endif()
endif()
