# Download and set up zip library

if(WIN32)
    fast_download_dependency(zip
            0.2.0
            63454fc97117e3f336b0fdc56308ddc7d3d4c2d775ee5d82916cf0d7f34e6bbc
            zip.lib
    )
elseif(APPLE)
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    fast_download_dependency(zip
            0.2.0
	    98b462abc0acdb8d7db3c80db647c48fb619d786986d188a120ebd74c201bfd2
            libzip.a
    )
else()
    fast_download_dependency(zip
            0.2.0
            6a1ed7b737265a50459b418bd5ee6f90463a2edeea73e8378519347ba333ab11
            libzip.a
    )
endif()
else()
    fast_download_dependency(zip
            0.2.0
            091a2f73263ebdfe36d0a489db8b758c9bac77169f133274513bc862a2f09eef
            libzip.a
    )
endif()
