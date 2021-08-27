## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled.")

    if(WIN32)
        fast_download_dependency(tensorflow
                2.4.0
                ff6b590025dd7b9a736987ff082ef0af20fe4f91a9b3fe846d25828d806a094b
        )
        set(TensorFlow_LIBRARIES tensorflow_cc.lib)
    else()
        fast_download_dependency(tensorflow
                2.4.0
                58c3988c5f5b95d49cdb87d3b42f665691fc6975ab2272ab581a31882364fab3
        )
        set(TensorFlow_LIBRARIES tensorflow_cc.so)
    endif()
endif()
