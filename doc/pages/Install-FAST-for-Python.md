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

importer = fast.ImageFileImporter\
    .create(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')

renderer = fast.ImageRenderer.create()\
    .connect(importer)

fast.SimpleWindow2D.create()\
    .connect(renderer)\
    .run()
```

To start using FAST, you might want to look at the [Python introduction tutorial](@ref python-tutorial-intro)
and the [Python examples page](@ref python-examples).

@m_class{m-block m-warning}

@par Running FAST on a remote server
    If you want to run FAST on a remote server <b>AND</b> visualize you need to use VirtualGL.
    Plain X forwarding (ssh -X) most likely will not work.
    [See this page for more info on how to use VirtualGL](@ref fast-remote-server).

Troubleshoot
-------------------

### NumPy Errors
If you get an error saying something like "ModuleNotFoundError: No module named 'numpy.core._multiarray_umath'" or "ImportError: numpy.core.multiarray failed to import". Try to completely remove numpy and reinstall:  
Run the following command **multiple times**, until it says "Cannot uninstall numpy..":

```bash
pip uninstall numpy
```

Then reinstall numpy:

```bash
pip install numpy
```

### Security warning on Mac OS X
When you try to run FAST, Mac will give you a security warning because FAST is not code signed (Apple charges money for this..).
Apple insists on giving you this warning for every binary in the release, you can add an exception for each of them
**or** you can disable the gatekeeper completely by opening your terminal and writing:

```bash
sudo spctl --master-disable
```

You can re-enable the gatekeeper later if you wish:

```bash
sudo spctl --master-enable
```

Uninstall
-------------------

```bash
pip uninstall pyfast
```

To delete the downloaded test data, and any cached data, simply delete the folders C:/ProgramData/FAST/ on windows, and /home/'your username'/FAST/ on Linux.