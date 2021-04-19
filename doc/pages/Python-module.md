Python Module {#python-module}
===============================

FAST has also support for Python. The module, called pyFAST, works with python 3 only. The Python wrapper is generated using [SWIG](http://swig.org/).

pyFAST can be installed using pip as follows, note however that you must have all the [requirements installed for the package to work](https://github.com/smistad/FAST/wiki/Requirements).
```bash
pip install pyfast
```


You can find examples on how to use FAST in python [here](https://github.com/smistad/FAST/wiki/Examples#python).

Building the Python wrappers
-------------------------------
The Python wrapper is a separate module which have to be enabled with the FAST_MODULE_Python CMake flag.

### Windows
Download [Python 3 64 bit](https://www.python.org/downloads/) and [SWIG >= 4](http://www.swig.org/download.html).
You can install multiple python versions and create a wheel for all of them using the FAST_Python_Version CMake option.

### Linux/Ubuntu

To build the Python wrappers you will need to install SWIG on your system. For Ubuntu linux you can install the SWIG package:
```bash
sudo apt-get install swig4.0
```
You also need python libs installed.
```bash
sudo apt-get install libpython3-dev # Python 3
```

### Building

CMake will try to automatically find SWIG on your system, and if it's found it will build the Python wrappers automatically. If CMake can't find SWIG after you installed it, set the SWIG_EXECUTABLE variable manually. On Ubuntu the executable is installed at /usr/bin/swig4.0.

The python module is integrated with numpy, thus requiring numpy to be installed:
```bash
# For both windows and linux:
pip install numpy
# For linux only:
pip install pylddwrap
```

Make sure you have built the entire FAST framework first:
```bash
cmake --build . --config Release -j8
```

You may then build the target named _fast, which will build the FAST python bindings.

### Building the Python wheel package

Building the target named python-wheel will create a python package, also a called a wheel in the folder build_folder/python/dist/

You can upload this package to pypi using twine:
```bash
twine upload -r testpypi pyFAST-xxxx.whl # For test pypi
twine upload pyFAST-xxxx.whl # For pypi
```

Troubleshoot
-------------------------------
For windows, you might get the error that numpy/arrayobject.h was not found. To fix this, create a symlink to the numpy include folder in the include folder of your python installation. To do this, open the command prompt with administration rights and type the following (assuming python is installed at C:\Python27):
```bash
mklink /D C:\Python27\include\numpy C:\Python27\Lib\site-packages\numpy\core\include\numpy
```

If you get several link errors like: "error LNK2019: unresolved external symbol __imp_PyType_IsSubtype referenced in function ...". Make sure you have downloaded the 64 bit version of python (see https://www.python.org/downloads/windows/).

On windows the Python paths in CMake may have to be set manually:
```
PYTHON_LIBRARY="C:\Python38\libs\python38.lib" 
PYTHON_INCLUDE_DIR="C:\Python38\include" 
```