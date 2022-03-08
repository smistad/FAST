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
		d2fbfcfb97cb1be55bba947e808dbbee1ed499cfe83d1cf139e50c1cc693be6f
	)
    else()
        fast_download_dependency(openvino
                2021.4.2
		6a47b44b4d80e41ef22d6df19b829e59b985c202b9ab596056570a6d1c65c21e
        )

    endif()
endif()
