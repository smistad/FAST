# Download and set up zlib

if(WIN32)
	fast_download_dependency(zlib
			2.2.5
			9bb7fd3961516aa9350d04a4835fa56b16bc5dcb4ec02ae6d18989744dfc2672
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
			2.2.5
			a4d79e28a925ca87a633a227d6dc948fce061e01da766985805db57a6cbea574
			libz.so
	)
endif()
