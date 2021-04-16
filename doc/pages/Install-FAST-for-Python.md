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
Then write the following python code, which will download a test dataset and display an ultrasound image on your screen:
```py
import fast

fast.downloadTestDataIfNotExists() # This will download the test data (~2GB) needed to run the example

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_0.mhd')

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
```
For more examples, see [this page](https://github.com/smistad/FAST/wiki/Examples#Python).

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