Install FAST for Ubuntu Linux {#install-ubuntu-linux}
===========================

Requirements
==================
**Make sure you have all the necessary requirements before installing FAST**:
- Ubuntu Linux 18.04 or newer.
- OpenGL: Usually installed along with your graphics driver.
- OpenCL: To install OpenCL on Linux, download an implementation depending on the CPU/GPU you have:    
    - **NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)  
    - **Intel** - Install the [OpenCL NEO driver](https://github.com/intel/compute-runtime/releases)  
    - **AMD** - Install the [ROCm stack](https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html)
    - If none of the above fits, you might want to check out the open source OpenCL implementation [Portable Computing Lanauge (PCOL)](http://portablecl.org), although reduced performance is likely.

Some [optional requirements](@ref requirements) are needed for video streaming and GPU neural network inference, these can be installed later.


Choose your installation method
==================

@m_div{m-button m-success} <a href="#python-linux">@m_div{m-big}Install FAST for Python @m_enddiv @m_div{m-small} If you wish to use FAST with Python using the pyFAST package. @m_enddiv </a> @m_enddiv

@m_div{m-button m-primary} <a href="#cpp-linux">@m_div{m-big}Install FAST for C++ @m_enddiv @m_div{m-small} If you wish to develop a C++ application using FAST. @m_enddiv </a> @m_enddiv

Python  {#python-linux}
=================

To install FAST for python (3.6 or newer), you may simply install FAST using pip, preferably using a clean python environment without conda/anaconda/similar:

```bash
pip install pyfast
```

To test that your installation works you can do the following from your terminal (powershell):

```bash
# Start python
python
```

Then run the following python code, which should display the FAST logo on your screen. If you can get an error, please see **[troubleshoot](#troubleshoot-linux)** section below.

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

C++ {#cpp-linux}
==============
To install FAST for C++ development, download the debian installer (fast_ubuntu18.04_X.X.X.deb) from the [release page](https://github.com/smistad/FAST/releases) and install the package as follows:

```bash
sudo dpkg -i fast_ubuntu18.04_X.X.X.deb
```

This will install FAST in the directory <b>/opt/fast/</b>.

To test if your FAST installation works, you can the following:

```bash
cd /opt/fast/bin/
./systemCheck
```

You should now see the FAST logo on your screen along with some technical information on OpenCL.
If you can get an error, please see **[troubleshoot](#troubleshoot-linux)** section below.

To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

Troubleshoot {#troubleshoot-linux}
==============

* Python
  * If you get an error saying something like **ModuleNotFoundError: No module named 'numpy.core._multiarray_umath'** or **ImportError: numpy.core.multiarray failed to import**. Try to completely remove numpy and reinstall:  
    Run the following command **multiple times**, until it says "Cannot uninstall numpy..": `pip uninstall numpy`
    Then reinstall numpy: `pip install numpy`
* C++
  * If the installer fails you can download .tar.xz file instead, and just extract it to anywhere on your drive.

Uninstall {#uninstall-linux}
==============

@m_class{m-block m-success}

@par Python
@parblock
```bash
pip uninstall pyfast
``` 
To delete the test data and any cached data, delete the folder /home/'your username'/FAST/ as well.
@endparblock

@m_class{m-block m-primary}

@par C++
@parblock
To uninstall FAST:
```bash
sudo apt remove fast
```
To delete the test data and any cached data, delete the folder /home/'your username'/FAST/ as well.
@endparblock
