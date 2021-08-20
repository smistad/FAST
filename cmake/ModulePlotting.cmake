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
                7ee607557e8401abec3cddcbcb49a3d63c3bd59461c231c65711978467971c77
                libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so libJKQTMathTextSharedLib_Release.so
        )
    endif()
endif()
