FAST Docker Containers {#fast-containers}
=================================
@tableofcontents

If you wish to use or build FAST in an isolated containerized environment you can use the following prebuilt containers
which comes with all dependencies including OpenCL and OpenGL preinstalled.

There are 3 main types of containers for FAST:
* **Library:** For using FAST as a library. This comes with the FAST debian package installed in /opt/fast/ that has the C++ headers and binaries included.
* **Python:** For using FAST with Python. This comes with pyfast (FAST Python package) installed.
* **Build:** For building FAST.

The containers can be found on [GitHub](https://github.com/orgs/FAST-Imaging/packages).

## Runtime containers

The runtime containers are used for running and using FAST.

### Python

The python containers have pyfast installed in a virtual environment which is activated on start.

Example of pulling the fast-python container with PoCL and Xvfb installed:

```bash
docker pull ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb
```

Example of running the container and starting python:

```bash
docker run -ti --rm ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb python
```

### Library

The library containers have the FAST debian package installed in /opt/fast/ and includes
C++ binaries, headers and tools.

Example of pulling the fast-library container with PoCL and Xvfb installed:

```bash
docker pull ghcr.io/fast-imaging/fast-library:4.14.1-pocl-xvfb
```

Example of running the container and executing the FAST systemCheck tool which prints system information:

```bash
docker run -ti --rm ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb systemCheck --no-gui
```

### OpenCL

FAST needs OpenCL to run. There are many OpenCL implementations and you should choose the one that fits
the hardware you plan to run FAST on:

* **pocl**: [Portable Computing Language](https://portablecl.org/) is an open source OpenCL implementation.
* **intel**: [Open-source OpenCL platform for Intel CPUs and GPUs](https://github.com/intel/compute-runtime). You have to add _--device=/dev/dri_ when running to get access to the intel processor.
* **nvidia**: OpenCL platform for NVIDIA GPUs. You have to add _--gpus=all_ when running to get access to the GPUs. 
Note that currently there are no prebuilt FAST containers for NVIDIA GPUs, however you can [build it yourself](https://github.com/FAST-Imaging/FAST-docker-containers/#nvidia-opencl).

### Visualization

FAST uses GLX for visualization and therefore needs an X server to render. So if you wish to render inside the docker
container you need to either:
* Give docker access to your X server.
* Use a container with X virtual framebuffer (xvfb) installed. In this case, rendering does not use hardware acceleration, but can run in a headless (no display) environment.

If you want interactive visualization, e.g. you want a GUI in which you can interact with the FAST visualizations, you need to either:
* Give docker access to your X server.
* Use Virtual GL (VGL). In this case, the rendering happens inside the docker container using Xvfb, but the rendered image is sent to your X server where it is displayed.

#### Giving the containers access to your X server
If you wish to render and display the GUI outside on your host, outside of the docker container, you can do
so by giving the docker container access to your X server which is done by:
- Provide the DISPLAY environment variable which is where the window should appear. By setting it to $DISPLAY it uses your current display.
- Mount the XAUTHORITY path and set the XAUTHORITY environment variable which is needed for docker to get access to your display.
- Mount /tmp/.X11-unix/ which is where the current X11 displays are located.

Example:
```bash
# Pull a docker image with PoCL and no X server:
docker pull ghcr.io/fast-imaging/fast-python:4.14.1-pocl-no_x

# Run the systemCheck tool inside the docker container:
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -e XAUTHORITY=$XAUTHORITY \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v $XAUTHORITY:$XAUTHORITY:ro \
    ghcr.io/fast-imaging/fast-python:4.14.1-pocl-no_x \
    systemCheck
```

#### Interactive visualization with VirtualGL
VirtualGL (VGL) and Xvfb is needed to achieve interactive visualization while rendering inside the docker.

VirtualGL will ensure that rendering is done using Xvfb inside the docker. While the rendered image
is sent to the host X server, and mouse and keyboard interactions are sent from the host X server to the xvfb server
inside the docker container.

When running you have to give the docker container access to your X server, which is done by:
- Provide the DISPLAY environment variable which is where the window should appear. By setting it to $DISPLAY it uses your current display.
- Mount the XAUTHORITY path and set the XAUTHORITY environment variable which is needed for docker to get access to your display.
- Mount /tmp/.X11-unix/ which is where the current X11 displays are located.

Example:
```bash
# Pull a docker image with PoCL, Xvfb and VGL installed:
docker pull ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb-vgl

# Run the systemCheck tool inside the docker container:
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -e XAUTHORITY=$XAUTHORITY \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v $XAUTHORITY:$XAUTHORITY:ro \
    ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb-vgl \
    systemCheck
```

#### Headless rendering
For headless rendering, e.g. rendering with no display, you only need a docker container with Xvfb installed.
In this case you cannot use a Window or GUI in FAST. But you can render to an image, and save that to disk.

Python example of headless rendering:
```bash
# Pull a docker image with PoCL and Xvfb installed:
docker pull ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb

# Run the docker container, mount the current directory and start python
docker run -ti --rm -v .:/output/ ghcr.io/fast-imaging/fast-python:4.14.1-pocl-xvfb python
```

Then you can try the following:
```python
import fast

importer = fast.ImageFileImporter\
    .create(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')

renderer = fast.ImageRenderer.create()\
    .connect(importer)

toImage = fast.RenderToImage.create()\
    .connect(renderer)

fast.ImageExporter.create('/output/headless_rendering.jpg')\
    .connect(toImage)\
    .run()
```
This should render the image inside the container, and store it as 'headless_rendering.jpg' in the current folder on your host.

## Build container

The build container is used for building, e.g. compiling, FAST.

```bash
docker pull ghcr.io/fast-imaging/fast-build:4.14.1
```

Running this container will checkout FAST from GitHub master branch and compile it:
```bash
docker run -ti ghcr.io/fast-imaging/fast-build:4.14.1
```

## Container source

If you wish to build the containers yourself, you can find the Dockerfiles and other associated files in
the [FAST-docker-containers repository on GitHub](https://github.com/FAST-Imaging/FAST-docker-containers).

## Apptainer

The FAST docker images can be converted to an Apptainer container image by first saving it to an OCI file using `docker save`
and then using the `apptainer build` command.

```bash
# Save a FAST docker image to .tar file:
docker save fast-image -o fast-image.tar
# Convert to apptainer sif image format:
sudo apptainer build fast_image.sif docker-archive://fast-image.tar
```