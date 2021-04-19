Build FAST on Mac OS X {#building-on-mac}
==========================

@m_class{m-note m-warning}

FAST on Mac OS X is not officially maintained anymore due to Apple's decision to drop support for both OpenCL and OpenGL.

This was tested on Mac OS X 10.12 Sierra.

**Install dependencies**

1. Download and install Xcode from the App store.
Open Xcode once to make sure it is set up.
Then run the following command in your terminal to install the command line tools:
   
```bash
xcode-select --install
```

2. Download and install git either from xcode or https://git-scm.com/download/mac

3. Download and install CMake [http://www.cmake.org/download/](http://www.cmake.org/download/)

4. FAST will download and build all other dependencies (Qt5, eigen, zlib) automatically when you build.

**Compile**

1. Download the FAST source code using git

```bash
git clone https://github.com/smistad/FAST.git
```

2. Open CMake and set the source directory to the /path/to/where/you/installed/FAST/ directory. Then specify a build directory. Press configure and choose Unix makefiles. Press generate.
3. Go to the build directory in your terminal and write "make -j8".

**Running the tests**

Next, you should run the tests to make sure the framework is working properly on your system. Instructions on how to do this can be found [here](https://github.com/smistad/FAST/wiki/Running-the-tests).
