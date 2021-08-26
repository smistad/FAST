# Download and set up zip library

if(WIN32)
    fast_download_dependency(zip
            0.2.0
            63454fc97117e3f336b0fdc56308ddc7d3d4c2d775ee5d82916cf0d7f34e6bbc
            zip.lib
    )
else()
    fast_download_dependency(zip
            0.2.0
            091a2f73263ebdfe36d0a489db8b758c9bac77169f133274513bc862a2f09eef
            libzip.a
    )
endif()
