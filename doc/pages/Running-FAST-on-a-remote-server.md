Running FAST on a remote server {#fast-remote-server}
========================================

FAST can run on a remote Linux server using SSH. [Install FAST and its dependencies for Linux](@ref install-ubuntu-linux).

**If you don't want to do any visualization, you don't have to do anything extra.** 
FAST will detect that there is no display to render and thus disable visualization (i.e. OpenGL) in FAST. 
If you try to do anything that requires OpenGL (rendering etc.) FAST will throw an exception.

However, if you want to do rendering and visualization with FAST on a remote headless server, you need to use OpenGL, X and GLX.
To do this, you either need Xvfb or VirtualGL depending on whether you want an interactive FAST window or not, see below for more information.

## Remote FAST visualization WITHOUT interactive window
**If you only want to render a visualization FAST, without interactive having an window, you can use Xvfb**.
Since X requires a display, which you don't necessarily have on a headless server, you
can use Xvfb (X virtual framebuffer) to create a virtual screen.

First install Xvfb:
```bash
sudo apt install xvfb
```
Then you can create a virtual screen with a given resolution and DISPLAY ID:
The DISPLAY ID should be unique, so if you have multiple users, and/or a real screen attached to the server.
Make sure you select a DISPLAY ID which is unused.
```bash
DISPLAY=:10 # Set the DISPLAY environment variable
Xvfb "$DISPLAY" -screen 0 1920x1080x24 & # You can change the resolution here if wanted
```

Now you should be able to render a FAST visualization on a headless remote server which you can then save to an image file
or display with another visualization program which can run with X forwarding, for instance matplotlib if you are using python.

## Examples
Make sure you have Xvfb running and the DISPLAY environment variable active in the terminal
you are running these examples from.

**Example of remote headless rendering to an image file with FAST using python:**
```python
import fast

importer = fast.ImageFileImporter.create(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')
renderer = fast.ImageRenderer.create().connect(importer)
visualization = fast.RenderToImage.create(width=1024).connect(renderer)
fast.ImageFileExporter.create('fast_remote_render.jpg')\
    .connect(visualization)\
    .run()
```

**Example of remote headless rendering with FAST and display using matplotlib:**
```python
import fast
import matplotlib.pyplot as plt

importer = fast.ImageFileImporter.create(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')
renderer = fast.ImageRenderer.create().connect(importer)
visualization = fast.RenderToImage.create(width=1024)\
    .connect(renderer)\
    .runAndGetOutputData()

plt.imshow(visualization)
plt.show()
```

## Remote FAST visualization WITH interactive window
**If you want to run FAST with interactive visualization on a remote server you need to use [VirtualGL](https://virtualgl.org/).** 
Indirect rendering over SSH with X forwarding (ssh -X) will not work because FAST uses modern OpenGL (3+), which indirect GLX does not support (however some have reported that modern Mesa OpenGL does support this).

### Setting up VirtualGL

**WARNING: Installing VirtualGL can mess up your system if not done correctly. Make sure you know what you are doing. Also, note that this guide has not been updated for a long time.**

Follow the instructions on installing VirtualGL from the [official VGL documentation](https://virtualgl.org/).

### Using FAST with VirtualGL
Instead of using the ssh command to connect to the server, connect with the following virtualgl command instead:
```bash
vglconnect -s user@domain
```
If you are on secure local area network, you can drop the -s parameter which can increase performance.
Then, when you want to run FAST, add vglrun to your command, e.g. to run the importImageFromFile example:
```bash
vglrun /opt/fast/bin/importImageFromFile
```
The same for python:
```bash
# Interactive python:
vglrun python
# Running a python script file:
vglrun python your_script.py
```

For more info, read the [official VirtualGL documentation](https://virtualgl.org/).
