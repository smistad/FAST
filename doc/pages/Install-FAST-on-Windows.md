Install FAST on Windows {#install-windows}
======================

Requirements
-----------------
To install FAST on Windows, first make sure you have all the necessary requirements installed:
- **OpenCL and OpenGL**: These are usually installed along with your graphics driver.
- [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe)

Some [optional requirements](@ref requirements) are needed for video streaming and GPU neural network inference, this can be installed later. 

Download and install
-----------------
Download and run a Windows installer (fast_windows_X.X.X.exe) from the [FAST release page](https://github.com/smistad/FAST/releases).
Windows might prompt you with a security warning, to proceed you must press "More info" followed by "Run anyway".
The installer will install FAST on your computer and the default location is **C:/Program Files/FAST/**.
The installer will also add start menu shortcuts.

Test
-----------------
To test if your FAST installation works, you can the following from your terminal:
```bash
cd 'C:/Program Files/FAST/fast/bin/'
./systemCheck.exe
```
<b>Or</b> go the Start menu -> FAST -> System Check

You should now see the FAST logo on your screen along with some technical information on OpenCL.
To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

Troubleshoot
-------------------

Windows might prompt you with a security warning when running the installer, to proceed you must press "More info" followed by "Run anyway".

If the installer fails you can download zip file instead, and just extract it to anywhere on your drive.

If you get an error that says something like "The application can't start because OpenCL.dll is missing from your computer". You have to install OpenCL. OpenCL should be included with your graphics driver (Intel/NVIDIA/AMD). You can try and search your harddrive for OpenCL.dll; if it is found, make sure its path is included in the PATH environment variable.

If you get an error that says something like "The application can't start because VCRUNTIME140_1.dll is missing from your computer". You have to install the [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).

Uninstall
-------------------

To uninstall FAST, simply run the uninstaller C:/Program Files/FAST/Uninstall.exe.
To delete the test data and any cached data, delete the folder C:/ProgramData/FAST/ as well.
