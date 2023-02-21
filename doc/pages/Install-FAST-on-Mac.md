Install FAST on macOS {#install-mac}
======================

Requirements
==================
**Make sure you have all the necessary requirements before installing FAST**:
- macOS 10.13 or newer.
- Install [homebrew](https://brew.sh/) if you don't already have it. Then, install the following packages using homebrew:  
```bash
brew install openslide libomp
```

Some [optional requirements](@ref requirements) are needed for video streaming, these can be installed later. 

Choose your installation method
==================

@m_div{m-button m-success} <a href="#python-mac">@m_div{m-big}Install FAST for Python @m_enddiv @m_div{m-small} If you wish to use FAST with Python using the pyFAST package. @m_enddiv </a> @m_enddiv

@m_div{m-button m-primary} <a href="#cpp-mac">@m_div{m-big}Install FAST for C++ @m_enddiv @m_div{m-small} If you wish to develop a C++ application using FAST. @m_enddiv </a> @m_enddiv

Python  {#python-mac}
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

Then run the following python code, which should display the FAST logo on your screen. If you can get an error, please see **[troubleshoot](#troubleshoot-mac)** section below.

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

C++ {#cpp-mac}
==============
To install FAST for C++ development, download an archive (fast_mac_X.X.X.tar.xz) from the [FAST release page](https://github.com/smistad/FAST/releases).
Extract the contents to your drive.

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

To test if your FAST installation works, you can the following from your terminal:
```bash
cd '/path/to/where/you/extracted/FAST/bin/'
./systemCheck
```

You should now see the FAST logo on your screen along with some technical information on OpenCL.
To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

Troubleshoot {#troubleshoot-mac}
================

Uninstall
================

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
To uninstall FAST, delete the extracted folder.
To delete the test data and any cached data, also delete the folder /Users/<your username>/FAST/.
