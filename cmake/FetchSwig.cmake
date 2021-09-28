# Download and set up swig
include(FetchContent)
set(FILENAME swig_4.0.2_${FAST_DEPENDENCY_TOOLSET}.tar.xz)

if(WIN32)
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=f5060e39b896c42236d418cb40b785cdc54201079f7b6323d344fcf186a2ccb2
    )
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
