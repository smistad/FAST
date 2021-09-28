# Download and set up swig
include(FetchContent)
set(FILENAME swig_4.0.2_${FAST_DEPENDENCY_TOOLSET}.tar.xz)

if(WIN32)
    FetchContent_Declare(
      swig
      URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL_NEW}/${FILENAME}
      URL_HASH SHA256=a4eea185e04634c8d9cb2e5655bef6bf68a921a3790dae09f7f7289f5b916e1b
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
