if(FAST_MODULE_Plotting)
    message("-- Enabling plotting module")
    if(WIN32)
        fast_download_dependency(jkqtplotter
                2020.10
                cf7459cc6767e3ba815e62ee0d7744d0cc18a454c87cb48f750ec03dd15c39df
                JKQTCommonSharedLib_Release.lib JKQTPlotterSharedLib_Release.lib JKQTFastPlotterSharedLib_Release.lib JKQTMathTextSharedLib_Release.lib
        )
    elseif(APPLE)
	if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        fast_download_dependency(jkqtplotter
                2020.10
		0d086f36ecb56411f52f711b38f131dd4b626ab266f23af63140f449f855b4cf
                libJKQTCommonSharedLib_Release.dylib libJKQTPlotterSharedLib_Release.dylib libJKQTFastPlotterSharedLib_Release.dylib libJKQTMathTextSharedLib_Release.dylib
	)
	else()
        fast_download_dependency(jkqtplotter
                2020.10
                feb27bb5e9ac07885deab9b76aa2f266cc209e7455f8d6407ddda57b4b602e1f
                libJKQTCommonSharedLib_Release.dylib libJKQTPlotterSharedLib_Release.dylib libJKQTFastPlotterSharedLib_Release.dylib libJKQTMathTextSharedLib_Release.dylib
                )
	endif()
    else()
        fast_download_dependency(jkqtplotter
                2020.10
                545bd83788c605d8650c5ecf4ce673fa44f4908e100af42001ed08de9bebfbd1
                libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so libJKQTMathTextSharedLib_Release.so
        )
    endif()
endif()
