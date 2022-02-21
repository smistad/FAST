if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        fast_download_dependency(openvino
                2021.4.2
		7268d5ab14366e2df017a955ca2aa95c7cbd9a6c690f45113a26fb86d9199714
        )
    elseif(APPLE)
        fast_download_dependency(openvino
                2021.4.2
		d2fbfcfb97cb1be55bba947e808dbbee1ed499cfe83d1cf139e50c1cc693be6f
	)
    else()
        fast_download_dependency(openvino
                2021.4.2
                9e3fd9733cfdfc1312bd436abfcff74992cfb5c33e09e36e25a627617c608fa3
        )

    endif()
endif()
