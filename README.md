![](https://github.com/smistad/FAST/wiki/images/fast_logo.png)

FAST (Framework for Heterogeneous Medical Image Computing and Visualization) is an open-source cross-platform framework with the main goal of making it easier to do processing and visualization of medical images on heterogeneous systems (CPU+GPU).

A detailed description of the framework design can be found [on the project wiki](https://github.com/smistad/FAST/wiki/Framework-Design) or in the research article:  
[FAST: framework for heterogeneous medical image computing and visualization](http://dx.doi.org/10.1007/s11548-015-1158-5).  
Erik Smistad, Mohammadmehdi Bozorgi, Frank Lindseth.  
International Journal of Computer Assisted Radiology and Surgery. February 2015.

Preprint of article can be downloaded from [here](http://www.eriksmistad.no/wp-content/uploads/FAST_framework_for_heterogeneous_medical_image_computing_and_visualization.pdf).
If you use FAST for research, please cite this article.

Getting started with FAST
-------------------------

[![Join the chat at https://gitter.im/smistad/FAST](https://badges.gitter.im/smistad/FAST.svg)](https://gitter.im/smistad/FAST?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

To setup and build the framework, see the instructions for your operating system:
* [Linux (Ubuntu)](https://github.com/smistad/FAST/wiki/Linux-instructions)
* [Windows](https://github.com/smistad/FAST/wiki/Windows-instructions)
* [Mac OS X](https://github.com/smistad/FAST/wiki/Mac-OS-X-instructions)

To start using the framework, see the [Getting started with FAST](https://github.com/smistad/FAST/wiki/Getting-started-with-FAST) guide or the [examples page](https://github.com/smistad/FAST/wiki/Examples).

Build status of development branch
---------------------
* Ubuntu Linux 16.04, Intel CPU, NVIDIA GPU - ![Build Status](https://jenkins.eriksmistad.no/job/Ubuntu_16.04_NVIDIA_Development/badge/icon)
* Ubuntu Linux 16.04, Intel CPU, AMD GPU - ![Build Status](https://jenkins.eriksmistad.no/job/Ubuntu_16.04_AMD_Development/badge/icon)
* Mac OS X 10.12, Intel CPU, NVIDIA GPU - ![Build Status](https://jenkins.eriksmistad.no/job/Mac_10.12_NVIDIA_Development/badge/icon)
* Windows 10, AMD CPU, AMD GPU - ![Build Status](https://jenkins.eriksmistad.no/job/Windows_10_AMD_Development/badge/icon)
* Windows 10, Intel CPU, NVIDIA GPU - ![Build Status](https://jenkins.eriksmistad.no/job/Windows_10_NVIDIA_Development/badge/icon)

![Surface mesh extracted from a large abdominal CT scan in about 100 ms using FAST and a modern GPU.](https://github.com/smistad/FAST/wiki/images/surface_extraction.png) ![Ultrasound image segmented using binary thresholding.](https://github.com/smistad/FAST/wiki/images/binary_thresholding.png)
