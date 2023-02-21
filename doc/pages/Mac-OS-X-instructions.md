Build FAST on macOS {#building-on-mac}
==========================
@tableofcontents

Install requirements
-------------------------

1. Download and install Xcode from the App store.
Then run the following command in your terminal to install the command line tools:
```bash
sudo xcode-select --install
```

2. Download and install CMake [http://www.cmake.org/download/](http://www.cmake.org/download/)

3. FAST will download all other dependencies (Qt5, eigen, zlib ++) automatically when you build.

Clone and Configure
--------------------

@m_class{m-block m-warning}

@par Warning
Don't put the FAST code in a path with spaces, this will currently break the build.

Clone the code using Git and configure the project using CMake:
```bash
git clone https://github.com/smistad/FAST.git
cd FAST
mkdir build
cd build
# Default cmake configuration. Options may be added like so: 
# cmake .. -DFAST_BUILD_TESTS=OFF -DFAST_BUILD_EXAMPLES=ON -DFAST_MODULE_TensorFlow=ON
# If you are building for Apple Silicon ARM64 processors you should set the following flag -DCMAKE_OSX_ARCHITECTURES="arm64"
cmake ..
```

FAST has several [optional modules](@ref build-modules) and build options. These are enabled using cmake options named FAST_MODULE_<Name> and FAST_BUILD_<Name>.
Here is a list of some options which might be useful:
* FAST_BUILD_TOOLS
* FAST_BUILD_EXAMPLES
* FAST_BUILD_TESTS
* FAST_BUILD_DOCS
* FAST_MODULE_TensorFlo (not supported on Apple Silicon)w
* FAST_MODULE_TensorRT (not supported on Mac)
* FAST_MODULE_OpenVIN (not supported on Apple Silicon)O
* FAST_MODULE_Dicom
* FAST_MODULE_WholeSlideImaging
* FAST_MODULE_OpenIGTLink
* FAST_MODULE_Clarius (not supported on Apple Silicon)
* FAST_MODULE_Python
* FAST_MODULE_HDF5
* FAST_MODULE_Plotting
* FAST_MODULE_RealSense (not supported on Mac)

Compile
-----------------------

This will build all configured targets:

```bash
make -j8 # Use 8 parallel jobs for compiling
```

Test
----------------------

To test that your build was successful, run the systemCheck application:

```bash
./bin/systemCheck
```

This should display the FAST logo and some FAST+OpenCL information.

Troubleshoot
-----------------------

Install
----------------------
Set CMAKE_INSTALL_PREFIX to where you want to install FAST on your drive.
Then run:

```bash
make -j8 install
```

Build package
----------------------
To build an archive package for macOS run:

```bash
make -j8 package
```

Build the Python bindings (pyFAST)
-----------------------
Configure cmake with Python enabled:
```bash
cmake .. -DFAST_MODULE_Python=ON
```
Then build the python-wheel target:
```bash
make -j8 python-wheel
```
The wheel will appear in the python/dist folder.
