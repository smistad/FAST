# Download dependencies required for various image compression
if(WIN32)
    fast_download_dependency(jpegxl
        0.11.0
        03e8f26bee81ca9a8186e09d9839b5176b81db5fd57324bc3007ebba9094cec0
        jxl.lib jxl_threads.lib
    )
elseif(APPLE)
else()
    fast_download_dependency(jpegxl
        0.11.0
        81bc657f986c274785ffbb19ecf0b207c6db20d2513569705bc31884b6fdc87f
        libjxl.so libjxl_threads.so
    )
endif()
