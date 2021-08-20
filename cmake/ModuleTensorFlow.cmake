## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled.")

    if(WIN32)
        fast_download_dependency(tensorflow
                2.4.0
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
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
