# Download and set up zlib

if(WIN32)
	fast_download_dependency(zlib
			1.2.9
			bf6971104e98a8ac64c8b172f01508e95b7e4fd81e427511b5531dd1b6376b29
			zlib.lib
			)
elseif(APPLE)
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
	fast_download_dependency(zlib
			1.2.9
			bf91823e4782458629b816c01fdf1f013d5767d977d6a35cc2fff1072082a33a
			libz.dylib
			)
else()
	fast_download_dependency(zlib
			1.2.9
			74f790268fe20c564345e404f5836df9e24d1a603075adf4581cf58a85cc4dba
			libz.dylib
			)
endif()
else()
	fast_download_dependency(zlib
			1.2.9
			72d5aa4d2e12266858ff668ad9e0d44e707a42112008f566777f9bea1a792274
			libz.so
			)
endif()
