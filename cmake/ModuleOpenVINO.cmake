if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2021.4.2
		fce4a6699f1a53a634645529048fe02fa6bb5128f6293de3778b6ceb4c682176
        )
    elseif(APPLE)
        fast_download_dependency(openvino
                2021.4.2
		07f30051cac0ef7816e05a62809bc50f4850dc73b35e8eb53dbc6355e5f8bfe5
	)
    else()
        fast_download_dependency(openvino
                2021.4.2
		6a47b44b4d80e41ef22d6df19b829e59b985c202b9ab596056570a6d1c65c21e
        )

    endif()
endif()
