if(FAST_MODULE_Clarius)
  message(STATUS "-- Clarius ultrasound module enabled.")
  if(WIN32)
    fast_download_dependency(clarius
            8.0.1
            fbb84b29154fbf58833025188dea2c139caa2a5c136a5f48469839f43b9f6e05
    )
  else()
    fast_download_dependency(clarius
            8.0.1
            603974b7a194fbe68eb6bfeb7d46201ea7c3f54c2b2b0721ddcd8b8d4aebab9e
    )
  endif()
endif()
