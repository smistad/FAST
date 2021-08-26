if(FAST_MODULE_Clarius)
  message(STATUS "-- Clarius ultrasound module enabled.")
  include(cmake/ExternalClarius.cmake)
endif()
