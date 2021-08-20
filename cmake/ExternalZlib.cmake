# Download and set up zlib

if(WIN32)
	fast_download_dependency(zlib
			1.2.9
			f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
			zlib.lib
			)
else()
	fast_download_dependency(zlib
			1.2.9
			9d26c12164262b1d20bdc3c6e2a37fa93d1398801ebf4f52076e5130a4a61296
			libz.so
			)
endif()
