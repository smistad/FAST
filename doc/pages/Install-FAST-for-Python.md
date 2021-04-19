Install FAST for Python {#install-python}
===========================

To install FAST for Python (3.6 or newer), first make sure you have all the necessary [requirements installed](@ref requirements).

Then you may simply install FAST using pip:

```bash
pip install pyfast
```

Alternatively, you can also download a Python wheel (pyFAST-X.X.X-cp36-abi3-X.whl) from the [release page](https://github.com/smistad/FAST/releases), and install it by:

```bash
pip install pyFAST-X.X.X-cp36-abi3-X.whl
```

To test that your installation works you can do the following:

```bash
# Start python
python
```

Then write the following python code, which should display the FAST logo on your screen.

```py
import fast

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
```

To start learning FAST, you should take a look at the [Python introduction tutorial](@ref python-tutorial-intro).

@m_class{m-block m-warning}

@par Running FAST on a remote server
    If you want to run FAST on a remote server <b>AND</b> visualize you need to use VirtualGL.
    Plain X forwarding (ssh -X) most likely will not work.
    [See this page for more info on how to use VirtualGL](@ref fast-remote-server).

Troubleshoot
-------------------

If you get an error saying something like "ModuleNotFoundError: No module named 'numpy.core._multiarray_umath'" or "ImportError: numpy.core.multiarray failed to import". Try to completely remove numpy and reinstall:  
Run the following command **multiple times**, until it says "Cannot uninstall numpy..":

```bash
pip uninstall numpy
```

Then reinstall numpy:

```bash
pip install numpy
```

Uninstall
-------------------

```bash
pip uninstall pyfast
```

To delete the downloaded test data, and any cached data, simply delete the folders C:/ProgramData/FAST/ on windows, and /home/'your username'/FAST/ on Linux.