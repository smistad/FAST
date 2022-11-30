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
		f3b93e5d53e3232ad5c737ee756825d64e6e6ad72988edff1036a5b172f2e997
	)
    else()
        fast_download_dependency(openvino
                2021.4.2
		6a47b44b4d80e41ef22d6df19b829e59b985c202b9ab596056570a6d1c65c21e
        )

    endif()
endif()
