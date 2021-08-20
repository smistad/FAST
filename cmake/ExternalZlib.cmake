# Download and set up zlib

if(WIN32)
	fast_download_dependency(zlib
			1.2.9
			bf6971104e98a8ac64c8b172f01508e95b7e4fd81e427511b5531dd1b6376b29
			zlib.lib
			)
else()
	fast_download_dependency(zlib
			1.2.9
			9d26c12164262b1d20bdc3c6e2a37fa93d1398801ebf4f52076e5130a4a61296
			libz.so
			)
endif()
