## FAST Use cmake file

# Enable C++ 11
set(CMAKE_CXX_STANDARD 11)

# Position independent code
if(${CMAKE_COMPILER_IS_GNUCXX})
    add_definitions("-fPIC")
endif()

add_definitions("-DFAST_CONFIG_LOCATION=\"${CMAKE_CURRENT_LIST_DIR}/../\"")

include_directories(${FAST_INCLUDE_DIRS})
link_directories (${FAST_LIBRARY_DIRS})
