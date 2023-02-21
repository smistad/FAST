# Download and set up swig
include(FetchContent)
set(FILENAME swig_4.0.2_${FAST_DEPENDENCY_TOOLSET}.tar.xz)

if(WIN32)
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=56c1f0eae1e25643cc98b2489a22e78c2970a04007d02849eb355e865384ad18
    )
elseif(APPLE)
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=407aeef24b8a88994a51e565d9d5f018abbae897a0f0f69ed288e190a8336c08
    )
else()
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=105be24d7967ef68e3a9763b2012f401bdc3dc009ccd06e1a9fd16edeca940d7
    )
endif()
else()
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=d0b84c72dff878ee18a03d580b0cc9786b37aadbac019912fca1c812755d9bea
    )
endif()
if(NOT swig_POPULATED)
    message("Downloading swig..")
    FetchContent_Populate(swig)
endif()
