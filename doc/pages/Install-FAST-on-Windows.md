Install FAST on Windows {#install-windows}
======================

To install FAST on windows, first make sure you have all the necessary [requirements installed](@ref requirements).

Then, download a windows installer (fast_windows_X.X.X.exe) from the [release page](https://github.com/smistad/FAST/releases).
The installer will install FAST on your computer. The default location is C:/Program Files/FAST/.
The installer will also add start menu shortcuts.

To test if your FAST installation works, you can the following:
* Download the test data (~2GB), the data will be downloaded to C:/ProgramData/FAST/. By **either** 
    * Start menu -> FAST -> Download Test Data.
    * Running the downloadTestData.exe executable in the C:/Program Files/FAST/fast/bin folder.
* Run an example:
    * Go to the folder C:/Program Files/FAST/fast/bin/
    * Run the importImageFromDisk.exe example which should display and ultrasound image on your screen.
    * If this works, run the other examples as well.

Troubleshoot
-------------------

If the installer fails you can download zip file instead, and just extract it to anywhere on your drive.

If you get an error that says something like "The application can't start because OpenCL.dll is missing from your computer". You have to install OpenCL. OpenCL should be included with your graphics driver (Intel/NVIDIA/AMD). You can try and search your harddrive for OpenCL.dll; if it is found, make sure its path is included in the PATH environment variable.

If you get an error that says something like "The application can't start because VCRUNTIME140_1.dll is missing from your computer". You have to install the [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).

Uninstall
-------------------

To uninstall FAST, simply run the uninstaller C:/Program Files/FAST/Uninstall.exe.
To delete the test data and any cached data, delete the folder C:/ProgramData/FAST/ as well.
