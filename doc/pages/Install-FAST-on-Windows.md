Install FAST on Windows {#install-windows}
======================

Requirements
======================
**Make sure you have all the necessary requirements before installing FAST**:
- OpenCL and OpenGL: These are usually installed along with your graphics driver.
- Download and install [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).

Some [optional requirements](@ref requirements) are needed for video streaming and GPU neural network inference, these can be installed later. 


Choose your installation method
==================

@m_div{m-button m-success} <a href="#python">@m_div{m-big}Install FAST for Python @m_enddiv @m_div{m-small} If you wish to use FAST with Python using the pyFAST package. @m_enddiv </a> @m_enddiv

@m_div{m-button m-primary} <a href="#cpp">@m_div{m-big}Install FAST for C++ @m_enddiv @m_div{m-small} If you wish to develop a C++ application using FAST. @m_enddiv </a> @m_enddiv


Python {#python}
======================

To install FAST for python (3.6 or newer), you may simply install FAST using pip, preferably using a clean python environment without conda/anaconda/similar:

```bash
pip install pyfast
```

To test that your installation works you can do the following from your terminal (powershell):

```bash
# Start python
python
```

Then run the following python code, which should display the FAST logo on your screen. If you can get an error, please see **[troubleshoot](#troubleshoot)** section below.

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

To start using FAST for python, you might want to look at the [Python introduction tutorial](@ref python-tutorial-intro)
and the [Python examples page](@ref python-examples).

@m_class{m-block m-warning}

@par Running FAST on a remote server
If you want to run FAST on a remote server <b>AND</b> visualize you need to use VirtualGL.
Plain X forwarding (ssh -X) most likely will not work.
[See this page for more info on how to use VirtualGL](@ref fast-remote-server).

C++ {#cpp}
======================
To install FAST for C++ development, download and run a Windows installer (fast_windows_X.X.X.exe) from the [FAST release page](https://github.com/smistad/FAST/releases).
Windows might prompt you with a security warning, to proceed you must press "More info" followed by "Run anyway".
The installer will install FAST on your computer and the default location is **C:/Program Files/FAST/**.
The installer will also add start menu shortcuts.

To test if your FAST installation works, you can the following from your terminal:
```bash
cd 'C:/Program Files/FAST/fast/bin/'
./systemCheck.exe
```
<b>Or</b> go the Start menu -> FAST -> System Check

If you can get an error, please see **[troubleshoot](#troubleshoot)** section below.

You should now see the FAST logo on your screen along with some technical information on OpenCL.
To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

Troubleshoot {#troubleshoot}
======================

* If you get an error that says something like **The application can't start because OpenCL.dll is missing from your computer**. You have to install OpenCL. OpenCL should be included with your graphics driver (Intel/NVIDIA/AMD). You can try and search your harddrive for OpenCL.dll; if it is found, make sure its path is included in the PATH environment variable.
* If you get an error that says something like **The application can't start because VCRUNTIME140_1.dll is missing from your computer**. You have to install the [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).
* Python
  * If you get an error saying something like **ModuleNotFoundError: No module named 'numpy.core._multiarray_umath'** or **ImportError: numpy.core.multiarray failed to import**. Try to completely remove numpy and reinstall:  
    Run the following command **multiple times**, until it says "Cannot uninstall numpy..": `pip uninstall numpy`
    Then reinstall numpy: `pip install numpy`
* C++
  * Windows might prompt you with a security warning when running the installer, to proceed you must press "More info" followed by "Run anyway".
  * If the installer fails you can download zip file instead, and just extract it to anywhere on your drive.

Uninstall
======================

@m_class{m-block m-success}

@par Python
@parblock 
```bash
pip uninstall pyfast
``` 
To delete the test data and any cached data, delete the folder C:/ProgramData/FAST/ as well.
@endparblock

@m_class{m-block m-primary}

@par C++
To uninstall FAST, simply run the uninstaller C:/Program Files/FAST/Uninstall.exe.
To delete the test data and any cached data, delete the folder C:/ProgramData/FAST/ as well.
