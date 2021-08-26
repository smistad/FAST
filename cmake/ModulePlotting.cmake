if(FAST_MODULE_Plotting)
    message("-- Enabling plotting module")
    if(WIN32)
        fast_download_dependency(jkqtplotter
                2020.10
                cf7459cc6767e3ba815e62ee0d7744d0cc18a454c87cb48f750ec03dd15c39df
                JKQTCommonSharedLib_Release.lib JKQTPlotterSharedLib_Release.lib JKQTFastPlotterSharedLib_Release.lib JKQTMathTextSharedLib_Release.lib
        )
    else()
        fast_download_dependency(jkqtplotter
                2020.10
                545bd83788c605d8650c5ecf4ce673fa44f4908e100af42001ed08de9bebfbd1
                libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so libJKQTMathTextSharedLib_Release.so
        )
    endif()
endif()
