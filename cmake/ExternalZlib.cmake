# Download and set up zlib

if(WIN32)
	fast_download_dependency(zlib
			1.2.9
			bf6971104e98a8ac64c8b172f01508e95b7e4fd81e427511b5531dd1b6376b29
			zlib.lib
			)
elseif(APPLE)
	fast_download_dependency(zlib
			1.2.9
			2006656a5b9d0e3ef80685ef9c9396c00b9e4814be750e47bef5a05d733bf3d4
			libz.dylib
			)
else()
	fast_download_dependency(zlib
			1.2.9
			72d5aa4d2e12266858ff668ad9e0d44e707a42112008f566777f9bea1a792274
			libz.so
			)
endif()
