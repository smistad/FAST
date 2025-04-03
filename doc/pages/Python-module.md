Python Module {#python-module}
===============================

FAST has also support for Python. The module, called pyFAST, works with python 3 only. The Python wrapper is generated using [SWIG](http://swig.org/).

pyFAST can be installed using pip as follows, note however that you must have all the requirements installed for the package to work.
```bash
pip install pyfast
```

Building the Python wrappers
-------------------------------
The Python wrapper is a separate module which have to be enabled with the FAST_MODULE_Python CMake flag.

### Windows
Download [Python 3 64 bit](https://www.python.org/downloads/).

### Linux/Ubuntu

You also need python libs installed.
```bash
sudo apt-get install libpython3-dev # Python 3
```

### Building

The python module is integrated with numpy, thus requiring numpy to be installed:
```bash
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
```bash
cmake --build . --config Release -j8 --target python-wheel
```

You can upload this package to pypi using twine:
```bash
twine upload -r testpypi pyFAST-xxxx.whl # For test pypi
twine upload pyFAST-xxxx.whl # For pypi
```

Troubleshoot
-------------------------------

If you get several link errors like: "error LNK2019: unresolved external symbol __imp_PyType_IsSubtype referenced in function ...". Make sure you have downloaded the 64 bit version of python (see https://www.python.org/downloads/windows/).

On windows the Python paths in CMake may have to be set manually:
```
PYTHON_LIBRARY="C:\Python38\libs\python38.lib" 
PYTHON_INCLUDE_DIR="C:\Python38\include" 
```