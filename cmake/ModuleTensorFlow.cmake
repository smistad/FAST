## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled.")

    if(WIN32)
        fast_download_dependency(tensorflow
                2.4.0
                ff6b590025dd7b9a736987ff082ef0af20fe4f91a9b3fe846d25828d806a094b
                tensorflow_cc.lib
        )
    else()
        fast_download_dependency(tensorflow
                2.4.0
                5d548ad173df78cc736ed50820b6a5af77764763d727c9a601da2f4a7c5b2f06
                libtensorflow_cc.so
        )
    endif()
endif()
