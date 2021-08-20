if(FAST_MODULE_Plotting)
    message("-- Enabling plotting module")
    if(WIN32)
        fast_download_dependency(jkqtplotter
                2020.10
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
                JKQTCommonSharedLib_Release.lib JKQTPlotterSharedLib_Release.lib JKQTFastPlotterSharedLib_Release.liba JKQTMathTextSharedLib_Release.lib
        )
    else()
        fast_download_dependency(jkqtplotter
                2020.10
                7ee607557e8401abec3cddcbcb49a3d63c3bd59461c231c65711978467971c77
                libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so libJKQTMathTextSharedLib_Release.so
        )
    endif()
endif()
