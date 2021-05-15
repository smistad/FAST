Install FAST for Ubuntu Linux {#install-ubuntu-linux}
===========================

Requirements
-----------------
To install FAST on Ubuntu Linux (18.04 or newer) make sure you have installed the requirements:
- OpenGL: Usually installed along with your graphics driver.
- OpenCL: To install OpenCL on Linux, download an implementation depending on the CPU/GPU you have:    
    - **NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)  
    - **Intel** - Install the [OpenCL NEO driver](https://github.com/intel/compute-runtime/releases)  
    - **AMD** - Install the [ROCm stack](https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html)
    - If none of the above fits, you might want to check out the open source OpenCL implementation [Portable Computing Lanauge (PCOL)](http://portablecl.org), although reduced performance is likely.

Some [optional requirements](@ref requirements) are needed for video streaming and GPU neural network inference, this can be installed later.

Download and install
-----------------
Then, download the debian installer (fast_ubuntu18.04_X.X.X.deb) from the [release page](https://github.com/smistad/FAST/releases) and install the package as follows:

```bash
sudo dpkg -i fast_ubuntu18.04_X.X.X.deb
```

This will install FAST in the directory <b>/opt/fast/</b>.

Test
-----------------
To test if your FAST installation works, you can the following:

```bash
cd /opt/fast/bin/
./systemCheck
```

You should now see the FAST logo on your screen along with some technical information on OpenCL.
To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

@m_class{m-block m-warning}

@par Running FAST on a remote server
If you want to run FAST on a remote server <b>AND</b> visualize you need to use VirtualGL.
Plain X forwarding (ssh -X) most likely will not work.
[See this page for more info on how to use VirtualGL](@ref fast-remote-server).

Troubleshoot
------------------

If the installer fails you can download .tar.xz file instead, and just extract it to anywhere on your drive.

Uninstall
------------------

To uninstall FAST:
```bash
sudo apt remove fast
```

To delete the test data and any cached data, delete the folder /home/'your username'/FAST/ as well.
