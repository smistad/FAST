## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled.")

    if(WIN32)
        fast_download_dependency(tensorflow
                2.4.0
                391eb42064c585ae03a746d5d13535d9660336f21906303c0b8f8c4f3af50cca
        )
        set(TensorFlow_LIBRARIES tensorflow_cc.lib)
    elseif(APPLE)
        fast_download_dependency(tensorflow
                2.4.0
                9cb561c8dd8b77423c49407f7b13478a507e8ceccac660829c36d3bd68f1ad64
        )
        set(TensorFlow_LIBRARIES libtensorflow_cc.dylib libtensorflow_framework.2.dylib)
    else()
        fast_download_dependency(tensorflow
                2.4.0
                58c3988c5f5b95d49cdb87d3b42f665691fc6975ab2272ab581a31882364fab3
        )
        set(TensorFlow_LIBRARIES tensorflow_cc.so)
    endif()
endif()
