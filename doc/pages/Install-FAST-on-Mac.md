Install FAST on Mac OS {#install-mac}
======================

Requirements
-----------------
To install FAST on Mac OS, first make sure you have all the necessary requirements:
- Mac OS X 10.13 or newer. Only intel (x86_64) architecture is supported atm.
- Install [homebrew](https://brew.sh/) if you don't already have it. Install the following packages using homebrew:  
```bash
brew install openslide libomp
```

Some [optional requirements](@ref requirements) are needed for video streaming, this can be installed later. 

Download and install
-----------------
Download an archive (fast_mac_X.X.X.tar.xz) from the [FAST release page](https://github.com/smistad/FAST/releases).
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

Test
-----------------
To test if your FAST installation works, you can the following from your terminal:
```bash
cd '/path/to/where/you/extracted/FAST/bin/'
./systemCheck
```

You should now see the FAST logo on your screen along with some technical information on OpenCL.
To start using FAST, you might want to look at the [C++ introduction tutorial](@ref cpp-tutorial-intro)
and the [C++ examples page](@ref cpp-examples).

Troubleshoot
-------------------

Uninstall
-------------------

To uninstall FAST, delete the extracted folder.
To delete the test data and any cached data, also delete the folder /Users/<your username>/FAST/.
